#include "pch.h"
#include "renderer.h"
#include <gml/include/math_util.h>
#include <gml/include/matrix.h>
namespace
{
	Vertex vertices[4] =
	{
		{ { -0.2f, -0.2f, 0.0f, 1.0f }},
		{ { -0.2f, +0.2f, 0.0f, 1.0f }},
		{ { +0.2f, +0.2f, 0.0f, 1.0f }},
		{ { +0.2f, -0.2f, 0.0f, 1.0f }},
	};

	Index indices[6] =
	{
		0, 1, 2,
		0, 2, 3,
	};
}
Renderer::Renderer(int width, int height)
{
	m_width = width;
	m_height = height;
	m_color_buffer = new gml::color4[width * height];

	m_triangle_count = sizeof(indices) / sizeof(Index) / 3;

	vertices[0].color = gml::color4::red;
	vertices[1].color = gml::color4::yellow;
	vertices[2].color = gml::color4::green;
	vertices[3].color = gml::color4::blue;

}
Renderer::~Renderer()
{
	delete[] m_color_buffer;
}

void Renderer::Render()
{
	ClearBuffer();
	VertexShader();
	Rasterization();
	PixelShader();
	OutputMerge();
}

void Renderer::ClearBuffer()
{
	for (int h = 0; h < m_height; h++)
	{
		for (int w = 0; w < m_width; w++)
		{
			int index = h * m_width + w;
			m_color_buffer[index] = m_clear_color;
		}
	}
}
void Renderer::VertexShader() {}

void Renderer::Rasterization()
{
	for (int triangle = 0; triangle < m_triangle_count; triangle++)
	{
		Index ia = indices[triangle * 3 + 0];
		Index ib = indices[triangle * 3 + 1];
		Index ic = indices[triangle * 3 + 2];

		gml::vec2 min = gml::min_combine(gml::vec2(vertices[ia].position), gml::vec2(vertices[ib].position));
		min = gml::min_combine(min, gml::vec2(vertices[ic].position));

		gml::vec2 max = gml::max_combine(gml::vec2(vertices[ia].position), gml::vec2(vertices[ib].position));
		max = gml::max_combine(max, gml::vec2(vertices[ic].position));

		int xmini = static_cast<int>((min.x * 0.5f + 0.5f) * m_width);
		int xmaxi = static_cast<int>((max.x * 0.5f + 0.5f) * m_width);
		int ymini = static_cast<int>((min.y * 0.5f + 0.5f) * m_height);
		int ymaxi = static_cast<int>((max.y * 0.5f + 0.5f) * m_height);

		gml::vec2 a = gml::vec2(vertices[ia].position);
		gml::vec2 ab = gml::vec2(vertices[ib].position) - a;
		gml::vec2 ac = gml::vec2(vertices[ic].position) - a;
		float det = gml::det22_t(ab, ac);
		if (gml::fequal(det ,0.0f))
		{
			continue;
		}
		float inv_det = det < 0 ? 1.0f / det : -1.0f / det;
		for (int y = ymini; y < ymaxi; y++)
		{
			for (int x = xmini; x < xmaxi; x++)
			{
				//ÖØÐÄ×ø±êËãuv
				gml::vec2 pa = gml::vec2(x*2.0f / m_width - 1.0f, y * 2.0f / m_height - 1.0f) - a;
				float u = gml::det22_t(pa, ac) * inv_det;
				float v = gml::det22_t(ab, pa) * inv_det;
				if (u <0.0f || v < 0.0f || u > 1.0f || u + v >1.0f)
				{
					continue;
				}
				int index = y * m_width + x;
				m_color_buffer[index] = vertices[ia].color * (1.0f - u - v) +
					vertices[ib].color * u +
					vertices[ic].color * v;
			}
		}
	}
}

void Renderer::PixelShader() {}
void Renderer::OutputMerge() {}

void Renderer::CopyBuffer(byte* buffer, int width, int height, int pitch)
{
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			int index = pitch * (height - h - 1) + w * 3;
			int u = (int)(w * 1.0f / width * m_width + 0.5f);
			int v = (int)(h * 1.0f / height * m_height + 0.5f);
			int src_index = v * m_width + u;
			//point sample
			gml::color4 color = m_color_buffer[src_index];

			unsigned int color32 = color.to_rgba();
			buffer[index + 0] = (color32 >> 16) & 0xFF;
			buffer[index + 1] = (color32 >> 8) & 0xFF;
			buffer[index + 2] = color32 & 0xFF;
		}
	}
}