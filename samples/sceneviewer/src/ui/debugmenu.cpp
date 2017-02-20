/*
	Debug menu source
*/

#include "debugmenu.h"
#include <tscore/system/info.h>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////

UIDebugMenu::UIDebugMenu(Application* app) :
	m_app(app)
{
	tsassert(m_app);
}

UIDebugMenu::~UIDebugMenu()
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UIDebugMenu::show(double dt)
{
	auto& env = m_app->getSystem();
	auto table = env.getCVarTable();
	auto render = env.getRenderModule();
	auto api = render->getApi();

	SRenderModuleConfiguration rendercfg;
	render->getConfiguration(rendercfg);

	Vector lightcolour;
	Vector ambientcolour;
	bool useDiffMap = false;
	bool useNormMap = false;
	bool useSpecMap = false;
	bool useDispMap = false;

	float attenConst = 0.0f;
	float attenLinear = 0.0f;
	float attenQuad = 0.0f;

	table->getVarVector3D("lightcolour", lightcolour);
	table->getVarVector3D("ambientcolour", ambientcolour);

	table->getVarBool("useDiffMap", useDiffMap);
	table->getVarBool("useNormMap", useNormMap);
	table->getVarBool("useSpecMap", useSpecMap);
	table->getVarBool("useDispMap", useDispMap);

	table->getVarFloat("attenConst", attenConst);
	table->getVarFloat("attenLinear", attenLinear);
	table->getVarFloat("attenQuad", attenQuad);

	//Create dialog
	ImGui::Begin("Debug");
	{
		if (ImGui::CollapsingHeader("Scene Variables"))
		{
			ImGui::ColorEdit3("light colour", (float*)&lightcolour);
			ImGui::ColorEdit3("ambient colour", (float*)&ambientcolour);

			ImGui::Checkbox("enable diffuse mapping", &useDiffMap);
			ImGui::Checkbox("enable normal mapping", &useNormMap);
			ImGui::Checkbox("enable specular mapping", &useSpecMap);
			ImGui::Checkbox("enable parallax occlusion mapping", &useDispMap);

			ImGui::InputFloat("constant attenuation", &attenConst);
			ImGui::InputFloat("linear attenuation", &attenLinear);
			ImGui::InputFloat("quadratic attenuation", &attenQuad);
		}

		if (ImGui::CollapsingHeader("Performance"))
		{
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			const uint64 framestep = 50;
			const uint64 n = 100;

			m_frametimes.push_back((float)dt);

			if (m_frametimes.size() > n)
				m_frametimes.pop_front();

			m_frameno++;
			m_frametime += dt;

			if ((m_frameno % framestep) == 0)
			{
				m_framerates.push_back((float)framestep / (float)m_frametime);
				m_frametime = 0.0;

				if (m_framerates.size() > n)
					m_framerates.pop_front();
			}

			if ((uint32)m_frametimes.size() > 0) ImGui::PlotHistogram("Frametimes", [](void* data, int idx)->float { return (1000 * ((UIDebugMenu*)data)->m_frametimes[idx]); }, this, (int)m_frametimes.size(), 0, 0, FLT_MIN, FLT_MAX, ImVec2(0, 30));
			if ((uint32)m_framerates.size() > 0) ImGui::PlotHistogram("FPS", [](void* data, int idx)->float { return (((UIDebugMenu*)data)->m_framerates[idx]); }, this, (int)m_framerates.size(), 0, 0, FLT_MIN, FLT_MAX, ImVec2(0, 30));

			double frametime_sum = 0.0;
			double frametime_sum_squared = 0.0f;

			for (uint64 i = 0; i < m_frametimes.size(); i++)
			{
				frametime_sum += (1000 * m_frametimes[i]);
				frametime_sum_squared += ((1000 * m_frametimes[i]) * (1000 * m_frametimes[i]));
			}

			double frame_average = frametime_sum / n;
			ImGui::Text(format("Frametime average : %ms", frame_average).c_str());

			double frame_variance = ((frametime_sum_squared - ((double)n * (frame_average * frame_average))) / ((double)n - 1));
			ImGui::Text(format("Frametime variance : %ms", frame_variance).c_str());
			ImGui::Text(format("Frametime deviation : %ms", sqrt(frame_variance)).c_str());

			SRenderStatistics stats;
			api->getDrawStatistics(stats);
			ImGui::Text(format("Draw calls : %", stats.drawcalls).c_str());

			SSystemMemoryInfo meminfo;
			getSystemMemoryInformation(meminfo);

			ImGui::Text(format("Memory usage: %B", meminfo.mUsed).c_str());
			ImGui::Text(format("Memory capacity: %B", meminfo.mCapacity).c_str());
			ImGui::Text(format("Memory usage: %", (float)meminfo.mUsed / meminfo.mCapacity).c_str());
		}

		if (ImGui::CollapsingHeader("Settings"))
		{
			//Update display mode
			auto displayModeIdx = (int)rendercfg.displaymode - 1;
			const char* displayModeStrings[] = { "windowed", "borderless", "fullscreen" };
			if (ImGui::Combo("display mode", &displayModeIdx, displayModeStrings, ARRAYSIZE(displayModeStrings)))
			{
				render->setDisplayConfiguration((EDisplayMode)(displayModeIdx + 1), 0, 0, SMultisampling(0));
			}

			//Update MSAA
			int msaaIdx = (int)log2(rendercfg.multisampling.count);
			const char* msaaItems[] = { "x1", "x2", "x4", "x8" };
			if (ImGui::Combo("MSAA", &msaaIdx, msaaItems, ARRAYSIZE(msaaItems)))
			{
				//Map the combo box index to the multisample level
				uint32 level = (1 << msaaIdx);
				render->setDisplayConfiguration(eDisplayUnknown, 0, 0, SMultisampling(level));
			}

			//Update resolution
			int resItems[] = { (int)rendercfg.width, (int)rendercfg.height };
			if (ImGui::InputInt2("resolution", resItems, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				tsprofile("w:% h:%", resItems[0], resItems[1]);
				render->setDisplayConfiguration(eDisplayUnknown, resItems[0], resItems[1], SMultisampling(0));
			}

			/*
			//Update to borderless mode
			bool is_borderless = (rendercfg.displaymode == EDisplayMode::eDisplayBorderless);
			bool is_fullscreen = (rendercfg.displaymode == EDisplayMode::eDisplayFullscreen);
			if (ImGui::Checkbox("Borderless", &is_borderless))
			{
			m_env->getGraphics()->setDisplayConfiguration((is_borderless) ? eDisplayBorderless : eDisplayWindowed, 0, 0, SMultisampling(0));
			}
			if (ImGui::Checkbox("Fullscreen", &is_fullscreen))
			{
			m_env->getGraphics()->setDisplayConfiguration((is_fullscreen) ? eDisplayFullscreen : eDisplayWindowed, 0, 0, SMultisampling(0));
			}
			*/
		}
	}
	ImGui::End();

	table->setVar("lightcolour", lightcolour);
	table->setVar("ambientcolour", ambientcolour);

	table->setVar("useDiffMap", useDiffMap);
	table->setVar("useNormMap", useNormMap);
	table->setVar("useSpecMap", useSpecMap);
	table->setVar("useDispMap", useDispMap);

	table->setVar("attenConst", attenConst);
	table->setVar("attenLinear", attenLinear);
	table->setVar("attenQuad", attenQuad);
}

/////////////////////////////////////////////////////////////////////////////////////////////////