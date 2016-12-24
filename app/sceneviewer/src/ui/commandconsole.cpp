/*
	Command console source
*/

#include "commandconsole.h"

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////

UICommandConsole::UICommandConsole(Application* app) :
	m_app(app)
{
	tsassert(m_app);
	ts::global::getLogger().addStream(this);
}

UICommandConsole::~UICommandConsole()
{
	ts::global::getLogger().detachStream(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UICommandConsole::show()
{
	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiSetCond_FirstUseEver);
	if (!ImGui::Begin("Console"))
	{
		ImGui::End();
		return;
	}

	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);

	ImGui::Separator();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
	{
		lock_guard<recursive_mutex>lk(m_lineBufferMutex);
		for (int i = 0; i < m_lineBuffer.size(); i++)
		{
			const SLine& line = m_lineBuffer[i];

			ImGui::PushStyleColor(ImGuiCol_Text, line.colour);
			ImGui::TextUnformatted(line.text.c_str());
			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(format(
					"Function: %\nFile: %\nLine: %",
					line.logmeta.function.str(),
					line.logmeta.file.str(),
					line.logmeta.line
				).c_str(), i);
			}
		}
	}

	if (m_lineScroll)
	{
		ImGui::SetScrollHere();
		m_lineScroll = false;
	}

	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();

	// Command-line
	if (ImGui::InputText("Input", m_readBuffer.str(), 4000, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, [](ImGuiTextEditCallbackData* data) { return static_cast<UICommandConsole*>(data->UserData)->textCallback(data); }, this))
	{
		char* input_end = m_readBuffer.str() + strlen(m_readBuffer.str());
		
		while (input_end > m_readBuffer.str() && input_end[-1] == ' ')
			input_end--; *input_end = 0;
		
		executeCommand(m_readBuffer.str());

		strcpy(m_readBuffer.str(), "");

		ImGui::SetScrollHere();
	}

	if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

	ImGui::End();
}

int UICommandConsole::textCallback(ImGuiTextEditCallbackData* data)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UICommandConsole::write(const SLogMessage& msg)
{
	//Convert timestamp into hours/minutes/seconds
	tm t;
	localtime_s(&t, &msg.timestamp);
	char timestr[200];
	strftime(timestr, sizeof(timestr), "%H:%M:%S", &t);

	//Fill line struct
	SLine line;
	line.colour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	line.text = format("[%] %", timestr, msg.message.str());
	line.logmeta = msg;

	//Set text colour depending on log level
	if (msg.level == ELogLevel::eLevelError)
		line.colour = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
	else if (msg.level == ELogLevel::eLevelWarn)
		line.colour = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
	if (msg.level == ELogLevel::eLevelProfile)
		line.colour = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);

	lock_guard<recursive_mutex>lk(m_lineBufferMutex);
	m_lineBuffer.push_back(line);
	m_lineScroll = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UICommandConsole::executeCommand(const char* command)
{
	string commandbuf(trim(command));
	
	auto pos = commandbuf.find_first_of(' ');
	string commandname(commandbuf.substr(0, pos));
	string commandargs;

	if (pos != string::npos)
	{
		commandargs = commandbuf.substr(pos, string::npos);
	}

	toLower(commandname);

	auto it = m_commandTable.find(commandname);

	if (it == m_commandTable.end())
	{
		tswarn("unable to find console command: \"%\"", commandname);
	}
	else
	{
		commandargs = trim(commandargs);
		auto f = it->second;
		if (f != nullptr)
		{
			//Print command to ui console
			SLine line;
			line.colour = ImVec4(1.0f, 0.64f, 0.0f, 1.0f);
			line.text += "# ";
			line.text += command;
			lock_guard<recursive_mutex>lk(m_lineBufferMutex);
			m_lineBuffer.push_back(line);
			m_lineScroll = true;

			SCommandConsoleCallbackArgs args;
			args.params = commandargs;
			f(args);
		}
	}
}

void UICommandConsole::setCommand(const char* command, const CommandConsoleCallback& callback)
{
	string buf(trim(command));
	toLower(buf);
	m_commandTable[buf] = callback;
}

/////////////////////////////////////////////////////////////////////////////////////////////////