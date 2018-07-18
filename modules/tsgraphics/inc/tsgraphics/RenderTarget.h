/*
	Render Target classes
*/

#pragma once

#include <array>

#include "Driver.h"

namespace ts
{
	/*
		Image Target Resource wrapper

		Represents an image that can be rendered to
	*/
	class ImageTarget : private RPtr<ResourceHandle>
	{
		//Base type
		using Base = RPtr<ResourceHandle>;

	public:

		/*
			Basic constructors
		*/
		ImageTarget() = default;
		ImageTarget(ImageTarget&& rhs) :
			Base((Base&&)std::move(rhs)),
			m_info(rhs.m_info)
		{}
		void operator=(ImageTarget&& rhs)
		{
			Base::swap((Base&&)std::move(rhs));
			std::swap(m_info, rhs.m_info);
		}

		//Create a colour render target
		static ImageTarget createColourTarget(
			RenderDevice* device,
			uint32 width,
			uint32 height,
			ImageFormat format,
			uint32 multisampleLevel = 1,
			bool shaderVisible = false
		);

		//Create a depth target
		static ImageTarget createDepthTarget(
			RenderDevice* device,
			uint32 width,
			uint32 height,
			ImageFormat format = ImageFormat::DEPTH16,
			uint32 multisampleLevel = 1
		);

		//Resize the target
		void resize(uint32 width, uint32 height);

		//Get image view
		ImageView getView()
		{
			ImageView view;
			view.image = Base::handle();
			view.count = 1;
			view.index = 0;
			view.type = ImageType::_2D;
			return view;
		}

		//Read image target properties
		uint32 width() const { return m_info.width; }
		uint32 height() const { return m_info.height; }
		ImageFormat format() const { return m_info.format; }
		uint32 multisampleLevels() const { return m_info.msLevels; }

		//Forward base methods - image handle can only be accessed through getView()
		using Base::device;
		using Base::null;
		using Base::operator bool;

	private:

		ImageResourceInfo m_info;
	};

	/*
		Render target wrapper class

		Represents a set of image resources that can be drawn to
	*/
	template<uint rtvCount = 1>
	class RenderTargets : private RPtr<TargetHandle>
	{
		//Base type
		using Base = RPtr<TargetHandle>;

	public:

		/*
			Basic constructors
		*/
		RenderTargets(RenderDevice* device = nullptr) : Base(device, TargetHandle()) {}
		RenderTargets(RenderTargets&& rhs) :
			Base(std::move((Base&&)*this)),
			m_outputs(rhs.m_outputs),
			m_depthOutput(rhs.m_depthOutput),
			m_viewport(rhs.m_viewport),
			m_scissor(rhs.m_scissor)
		{}

		void operator=(RenderTargets&& rhs)
		{
			Base::swap(rhs);
			m_outputs = rhs.m_outputs;
			m_depthOutput = rhs.m_depthOutput;
			m_viewport = rhs.m_viewport;
			m_scissor = rhs.m_scissor;
		}

		//Attach an output image to the given slot
		void attach(uint32 idx, const ImageView& view) { m_outputs[idx] = view; invalidate(); }
		//Attach a depth image
		void attachDepth(const ImageView& view) { m_depthOutput = view; invalidate(); }

		//Set viewport/scissor properties
		void setViewport(const Viewport& viewport) { m_viewport = viewport; invalidate(); }
		void setScissor(const Viewport& scissor) { m_scissor = scissor; invalidate(); }

		//Access handle - create target lazily
		TargetHandle handle()
		{
			if (m_dirty && Base::device() != nullptr)
			{
				TargetCreateInfo info;
				info.attachments = m_outputs.data();
				info.attachmentCount = m_outputs.size();
				info.depth = m_depthOutput;
				info.scissor = m_scissor;
				info.viewport = m_viewport;

				Base::swap(Base::device()->createTarget(info, Base::release()));

				m_dirty = false;
			}

			return Base::handle();
		}

		//Read render target properties
		const ImageView* attachedOutputs() const { return m_outputs; }
		size_t attachedOutputCount() const { return rtvCount; }
		const ImageView& attachedDepthOutput() const { return m_depthOutput; }
		const Viewport& viewport() const { return m_viewport; }
		const Viewport& scissor() const { return m_scissor; }

		//Forward base methods
		using Base::device;
		using Base::null;
		using Base::operator bool;

	private:

		void invalidate() { m_dirty = true; }

		bool m_dirty = true;

		std::array<ImageView, rtvCount> m_outputs;
		ImageView m_depthOutput;

		Viewport m_viewport;
		Viewport m_scissor;
	};
}
