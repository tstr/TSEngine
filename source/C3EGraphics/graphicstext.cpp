/*
	Text rendering source
*/

#include "pch.h"

#include <C3E\core\memory.h>
#include <C3E\core\time.h>
#include <C3E\gfx\abi\graphicsabi.h>
#include <C3E\gfx\graphicstext.h>
#include <C3E\filesystem\filexml.h>

//Freetype-gl
#include "ft\texture-atlas.h"
#include "ft\texture-font.h"
#include "ft\distance-field.h"

using namespace C3E;
using namespace ftgl;
using namespace std;

LINK_LIB("freetype.lib")

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct TextFactory::Impl
{
	Graphics* m_renderer = nullptr;

	ftgl::texture_atlas_t* m_atlas = nullptr;
	ftgl::texture_font_t* m_font = nullptr;

	unique_ptr<Texture> m_texture;
	unique_ptr<GraphicsBuffer> m_vertexBuffer;
	unique_ptr<GraphicsBuffer> m_indexBuffer;

	vector<Index> m_indices;
	vector<Vertex> m_vertices;

	uint32 m_resW = 0;
	uint32 m_resH = 0;

	bool m_active = false;

	Impl(Graphics* r, const char* filename, uint32 pointscale) :
		m_renderer(r)
	{
		C3E_ASSERT(m_renderer);

		Stopwatch sw;

		cout << "Loading font from file: """ << filename << """\n";

		sw.Start();
		if (!FileSystem::FileExists(filename))
		{
			cerr << "ERROR unable to find file: """ << filename << """\n";
			throw exception(__FUNCTION__);
		}

		const size_t resW = 1024;
		const size_t resH = 1024;

		//const char text[] = "test";
		const char * cache = " !\"#$%&'()*+,-./0123456789:;<=>?"
			"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
			"`abcdefghijklmnopqrstuvwxyz{|}~";

		m_atlas = texture_atlas_new(resW, resH, 1);

		m_font = texture_font_new_from_file(m_atlas, (float)pointscale, filename);
		if (!m_font)
			cerr << "Unable to load font from file: """ << filename << """\n";

		texture_font_load_glyphs(m_font, cache);
		
		cout << "Computing distance field...\n";

		//Generate distance field
		byte* map = make_distance_mapb(m_atlas->data, (uint32)m_atlas->width, (uint32)m_atlas->height);
		memcpy(m_atlas->data, map, m_atlas->width* m_atlas->height * sizeof(byte));
		free(map);

		cout << "Distance field computed.\n";

		ABI::ShaderResourceData data;
		data.mem = m_atlas->data;
		data.memPitch = (uint32)(m_atlas->depth * m_atlas->width);
		data.memSlicePitch = 0;

		ABI::ShaderResourceDescriptor desc;
		desc.texformat = ETextureFormat::FormatByte;
		desc.textype = ETextureType::TypeTexture2D;
		desc.height = (uint32)m_atlas->height;
		desc.width = (uint32)m_atlas->width;

		m_texture.reset(new Texture(m_renderer, m_renderer->api()->CreateShaderResource(&data, desc)));
		sw.Stop();
		cout << "TrueType font loaded successfully. Elapsed time - " << sw.DeltaTime() << "ms";
	}

	~Impl()
	{
		texture_atlas_delete(m_atlas);
		texture_font_delete(m_font);
	}

	void Begin(uint32 resW, uint32 resH)
	{
		m_resW = resW;
		m_resH = resH;

		m_vertices.clear();
		m_indices.clear();

		m_active = true;
	}

	void DrawString(const char* str, int32 xoffset, int32 yoffset, Vector colour, float scale)
	{
		if (!m_active)
			throw exception(__FUNCTION__);

		vec2 origin = { 0, 0 };
		origin.x = -(((float)m_resW / 2) - xoffset);
		origin.y = (((float)m_resH / 2) - yoffset - (m_font->height * scale));
		vec2 pen = origin;

		Index vertex_step = (Index)m_vertices.size();

		for (size_t i = 0; i < strlen(str); ++i)
		{
			if (str[i] == '\n')
			{
				pen.y -= ((m_font->ascender - m_font->descender) + m_font->linegap) * scale;
				pen.x = origin.x;
				continue;
			}

			texture_glyph_t *glyph = texture_font_get_glyph(m_font, (const char*)str + i);
			
			if (glyph)
			{
				float kerning = 0.0f;
				if (i > 0)
				{
					kerning = texture_glyph_get_kerning(glyph, str + i + 1);
				}
				pen.x += kerning * scale;

				auto x0 = pen.x + ((float)glyph->offset_x * scale);
				auto y0 = pen.y + ((float)glyph->offset_y * scale);
				auto x1 = x0 + ((float)glyph->width * scale);
				auto y1 = y0 - ((float)glyph->height * scale);

				//Texture coordinates must be reversed for left handed coordinate system (s0->s1 t0->t1)
				//U texture coords
				float s0 = glyph->s1;
				float s1 = glyph->s0;
				//V texture coords
				float t0 = glyph->t1;
				float t1 = glyph->t0;

				Vertex v0, v1, v2, v3;

				//Matrix M = Matrix::CreateScale(scale);
				Matrix M;

				//0, 1, 2, 0, 2, 3 
				if (m_indices.size() > 0)
					m_indices.push_back(0 + vertex_step); //repeat first vertex of first strip

				m_indices.push_back(0 + vertex_step);
				m_indices.push_back(1 + vertex_step);
				m_indices.push_back(2 + vertex_step);

				m_indices.push_back(0 + vertex_step);
				m_indices.push_back(2 + vertex_step);
				m_indices.push_back(3 + vertex_step);

				m_indices.push_back(3 + vertex_step); //repeat last vertex of first strip

				//v0.set((uint32)VertexAttributeIndex::Position, Vector(x0, y0, 0));
				v0.set((uint32)VertexAttributeIndex::Position, Matrix::Transform(Vector(x1, y1, 0), M));
				v0.set((uint32)VertexAttributeIndex::Texcoord, Vector(s0, t0));
				v0.set((uint32)VertexAttributeIndex::Colour, colour);
				m_vertices.push_back(v0);

				//v1.set((uint32)VertexAttributeIndex::Position, Vector(x0, y1, 0));
				v1.set((uint32)VertexAttributeIndex::Position, Matrix::Transform(Vector(x1, y0, 0), M));
				v1.set((uint32)VertexAttributeIndex::Texcoord, Vector(s0, t1));
				v1.set((uint32)VertexAttributeIndex::Colour, colour);
				m_vertices.push_back(v1);

				//v2.set((uint32)VertexAttributeIndex::Position, Vector(x1, y1, 0));
				v2.set((uint32)VertexAttributeIndex::Position, Matrix::Transform(Vector(x0, y0, 0), M));
				v2.set((uint32)VertexAttributeIndex::Texcoord, Vector(s1, t1));
				v2.set((uint32)VertexAttributeIndex::Colour, colour);
				m_vertices.push_back(v2);

				//v3.set((uint32)VertexAttributeIndex::Position, Vector(x1, y0, 0));
				v3.set((uint32)VertexAttributeIndex::Position, Matrix::Transform(Vector(x0, y1, 0), M));
				v3.set((uint32)VertexAttributeIndex::Texcoord, Vector(s1, t0));
				v3.set((uint32)VertexAttributeIndex::Colour, colour);
				m_vertices.push_back(v3);

				vertex_step += 4; //incremenet offset by number of vertices per quad

				pen.x += glyph->advance_x * scale;
				pen.y += glyph->advance_y * scale;
			}
		}
	}

	void End()
	{
		//m_numIndices = (uint32)indices.size();
		m_vertexBuffer.reset(new GraphicsBuffer(m_renderer, make_buffer_from_vector(m_vertices), EResourceType::TypeVertexBuffer));
		m_indexBuffer.reset(new GraphicsBuffer(m_renderer, make_buffer_from_vector(m_indices), EResourceType::TypeIndexBuffer));

		m_active = false;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextFactory::TextFactory(Graphics* r, const char* f, uint32 p) :
	pImpl(new Impl(r, f, p))
{
}

TextFactory::~TextFactory()
{
	if (pImpl)
		delete pImpl;
}

const EffectResource* TextFactory::GetTextureAtlas() const
{
	return pImpl->m_texture.get();
}

void TextFactory::Begin(uint32 resW, uint32 resH)
{
	pImpl->Begin(resW, resH);
}

void TextFactory::DrawString(const char* str, int32 x, int32 y, Vector c, float scale)
{
	pImpl->DrawString(str, x, y, c, scale);
}

void TextFactory::End()
{
	pImpl->End();
}

uint32 TextFactory::GetBuffers(GraphicsBuffer*& vertexbuffer, GraphicsBuffer*& indexbuffer) const
{
	vertexbuffer = pImpl->m_vertexBuffer.get();
	indexbuffer = pImpl->m_indexBuffer.get();
	return (uint32)pImpl->m_indices.size();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////