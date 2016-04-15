/*
	Text renderer
*/

#include "graphics.h"

namespace C3E
{
	class C3E_GFX_API TextFactory
	{
	private:

		struct Impl;
		Impl* pImpl = nullptr;

	public:

		TextFactory(Graphics* fx, const char* filename, uint32 pointscale);
		~TextFactory();

		const EffectResource* GetTextureAtlas() const;

		void Begin(uint32 resW, uint32 resH);
		void DrawString(const char* str, int32 xoffset, int32 yoffset, Vector colour = Vector(1, 1, 1, 1), float scale = 0.5f);
		void End();

		uint32 GetBuffers(GraphicsBuffer*& vertexbuffer, GraphicsBuffer*& indexbuffer) const;
	};
}