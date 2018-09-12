/*
    3D Scene Graphics class
*/

#pragma once

#include <tsgraphics/Graphics.h>
#include <tsgraphics/Buffer.h>

#include "Renderable.h"
#include "ShaderConstants.h"
#include "Material.h"
#include "MaterialManager.h"

#include "ShadowPass.h"

namespace ts
{
	enum LightSource
	{
		LIGHT0,
		LIGHT1,
		LIGHT2,
		LIGHT3
	};

	class SceneRender
	{
	private:

		GraphicsSystem* m_gfx;
		MaterialManager m_materialManager;

		RenderTargets<> m_targets;

		Buffer m_perMesh;
		Buffer m_perScene;

		ShadowPass m_shadowPass;

		struct RenderItem
		{
			Matrix transform;
			const Renderable* item;
		};

		std::vector<RenderItem> m_renderQueue;

		/*
			Properties
		*/
		Vector m_ambientColour;
		Matrix m_viewMatrix;
		Matrix m_projMatrix;
		RGBA m_directLightColour;
		Vector m_directLightDir;
		DynamicLight m_dynamicLights[MAX_LIGHTS];

	public:

		SceneRender() {}
		SceneRender(const SceneRender&) = delete;

		SceneRender(
			GraphicsSystem* graphics
		);

		/*
			Set global constants
		*/

		void setCameraView(const Matrix& view) { m_viewMatrix = view; }
		void setCameraProjection(const Matrix& proj) { m_projMatrix = proj; }
		void setCamera(const Matrix& view, const Matrix& proj) { setCameraView(view); setCameraProjection(proj); }
		void setAmbientColour(const Vector& ambient) { m_ambientColour = ambient; }

		void setDirectionalLightColour(RGBA colour) { m_directLightColour = colour; }
		void setDirectionalLightDir(const Vector& dir) { m_directLightDir = dir; }

		using LightID = uint32;

		void enableDynamicLight(LightID light) { m_dynamicLights[light].enabled = true; }
		void disableDynamicLight(LightID light) { m_dynamicLights[light].enabled = false; }

		void setLightColour(LightID light, RGBA colour) { m_dynamicLights[light].colour = Vector(colour); }
		void setLightPosition(LightID light, const Vector& pos) { m_dynamicLights[light].pos = pos; }

		void setLightAttenuation(LightID light, float quadratic, float linear, float constant)
		{
			m_dynamicLights[light].attConstant = constant;
			m_dynamicLights[light].attLinear = linear;
			m_dynamicLights[light].attQuadratic = quadratic;
		}

		//Create a renderable
		Renderable createRenderable(const Mesh& mesh, const PhongMaterial&);

		//Draw a renderable
		void draw(const Renderable& item, const Matrix& transform)
		{
			m_renderQueue.emplace_back(RenderItem{ transform, &item });
		}

		void update();
	};
}
