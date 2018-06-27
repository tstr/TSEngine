/*
    Forward rendering pipeline
*/

#pragma once

#include <vector>
#include <unordered_map>

#include <tscore/path.h>
#include <tsgraphics/Graphics.h>
#include <tsgraphics/BindingSet.h>
#include <tsgraphics/Buffer.h>
#include <tsgraphics/Shader.h>
#include <tsgraphics/Model.h>
#include <tsgraphics/Image.h>

#include "ForwardRenderConstants.h"

namespace ts
{
    /*
        Material info:
        
        - Properties (ambient/diffuse/specular etc.)
        - Images
		- Mesh attribute layout
    */
    struct MaterialCreateInfo
    {
        using PathMap = std::unordered_map<String, Path>;
        PathMap images;
        
		MaterialConstants constants;
    };

	struct MeshInfo
	{
		Mesh data;
		VertexTopology topology;
		const VertexAttributeMap* attributeMap;
	};


    struct MaterialInstance
    {
        BindingSet<ImageView> images;
        Buffer buffer;
    };

    struct Renderable
    {
		MaterialInstance mat;

        RPtr<PipelineHandle> pso;
        RPtr<ResourceSetHandle> resources;

        RPtr<CommandHandle> draw;

		/*
		Renderable() {}
		Renderable(Renderable&& r)
		{
			std::swap(mat, r.mat);
			std::swap(pso, r.pso);
			std::swap(resources, r.resources);
			std::swap(draw, r.draw);
		}
		*/
    };

    class ForwardRenderer
    {
    public:

		ForwardRenderer() {}
		ForwardRenderer(const ForwardRenderer&) = delete;

		ForwardRenderer(GraphicsSystem* gfx);

		//Set global constants
		void setCameraView(const Matrix& view);
		void setCameraProjection(const Matrix& proj);

        //Create renderable item
        Renderable createRenderable(const MeshInfo& mesh, const MaterialCreateInfo&);

        //Draw a renderable item
        void draw(const Renderable& item, const Matrix& transform);


		void begin();
		void end();

    private:

		///////////////////////////////////////////////////////////////////////////////

		GraphicsSystem* m_gfx = nullptr;

		SceneConstants m_perSceneConst;
		MeshConstants m_perMeshConst;

		Buffer m_perMesh;
		Buffer m_perScene;

		ShaderProgram m_shader;

		RPtr<ResourceHandle> m_depthBuffer;
		RPtr<TargetHandle> m_target;

		std::vector<Image> m_imageCache;

		///////////////////////////////////////////////////////////////////////////////

		void preloadShaders();
		void loadTargets();
		void loadMaterial(MaterialInstance& mat, const MaterialCreateInfo& info);

        RPtr<ResourceSetHandle> makeResourceSet(const Mesh& mesh, const MaterialInstance& mat);
        RPtr<PipelineHandle> makePipeline(const MeshInfo& mesh, const MaterialInstance& mat);
    };
}
