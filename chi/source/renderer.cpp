#include "pch.h"
#include "renderer.h"
#include <gml/include/math_util.h>
#include <gml/include/matrix.h>
#include <vector>

struct Fragment
{
	int x;
	int y;
	gml::color4 color;
	float z;
};

namespace
{
	const float FOV = static_cast<float>(gml::PI * 0.5);
	const float size = 1.00f;
	Vertex vertices[8] =
	{
		{ { -size, -size, 0.0f, 1.0f } },
		{ { -size, +size, 0.0f, 1.0f } },
		{ { +size, +size, 0.0f, 1.0f } },
		{ { +size, -size, 0.0f, 1.0f } },
		{ { 0.0f, -size, -size, 1.0f } },
		{ { 0.0f, +size, -size, 1.0f } },
		{ { 0.0f, +size, +size, 1.0f } },
		{ { 0.0f, -size, +size, 1.0f } },
	};

	Index indices[] =
	{
		4, 5, 6,
		4, 6, 7,
		0, 1, 2,
		0, 2, 3,
	};

	struct V2F
	{
		gml::vec4 sv_position;
		gml::color4 color;
	};

	std::vector<V2F> inner_vertices;
	std::vector<Fragment> fragments;
	std::vector<Fragment> out_fragments;
}



Renderer::Renderer(int width, int height)
{
	m_width = width;
	m_height = height;
	m_color_buffer = new gml::color4[width * height];
	m_depth_buffer = new float[width * height];

	m_vertex_count = sizeof(vertices) / sizeof(Vertex);
	m_triangle_count = sizeof(indices) / sizeof(Index) / 3;

	vertices[0].color = gml::color4::red;
	vertices[1].color = gml::color4::yellow;
	vertices[2].color = gml::color4::green;
	vertices[3].color = gml::color4::blue;
	vertices[4].color = gml::color4::red;
	vertices[5].color = gml::color4::yellow;
	vertices[6].color = gml::color4::green;
	vertices[7].color = gml::color4::blue;

	m_mat_world = gml::mat44::scale(1.0f,0.5f,1.0f);
	m_mat_view = gml::mat44::look_at(gml::vec3(0.0f, 0.0f, -1.0f), gml::vec3::zero, gml::vec3(0, 1, 0));
	m_mat_proj = gml::mat44::perspective(FOV, m_width * 1.0f / m_height, 1.0f, 1000.0f);
	
	m_mat_mv = m_mat_view * m_mat_world;
	m_mat_vp = m_mat_proj * m_mat_view;
	m_mat_mvp = m_mat_proj * m_mat_mv;
}

Renderer::~Renderer()
{
	delete[] m_color_buffer;
	delete[] m_depth_buffer;
}

void Renderer::Render()
{
	static float r = 0.0f;
	r += 0.025f;
	const float radius = 2.5f;

	m_mat_view = gml::mat44::look_at(gml::vec3(sin(r)*radius, 2.0f, cos(r)*radius), gml::vec3::zero, gml::vec3(0, 1, 0));

	m_mat_mv = m_mat_view * m_mat_world;
	m_mat_vp = m_mat_proj * m_mat_view;
	m_mat_mvp = m_mat_proj * m_mat_mv;

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
			m_clear_color.r = h*0.2f / m_height;
			m_color_buffer[index] = m_clear_color;
			m_depth_buffer[index] = 1.0f;
		}
	}

	fragments.clear();
}
void Renderer::VertexShader() 
{
	inner_vertices.resize(m_vertex_count);
	for (int i = 0; i < m_vertex_count; i++)
	{
		inner_vertices[i].sv_position = m_mat_mvp * vertices[i].position;
		float inv_z = 1.0f / inner_vertices[i].sv_position.w;
		inner_vertices[i].sv_position.x *= inv_z;
		inner_vertices[i].sv_position.y *= inv_z;
		inner_vertices[i].sv_position.z *= inv_z;
		inner_vertices[i].color = vertices[i].color;
	}
}

void Renderer::Rasterization()
{
	for (int triangle = 0; triangle < m_triangle_count; triangle++)
	{
		Index ia = indices[triangle * 3 + 0];
		Index ib = indices[triangle * 3 + 1];
		Index ic = indices[triangle * 3 + 2];

		gml::vec2 min = gml::min_combine(gml::vec2(inner_vertices[ia].sv_position), gml::vec2(inner_vertices[ib].sv_position));
		min = gml::min_combine(min, gml::vec2(inner_vertices[ic].sv_position));

		gml::vec2 max = gml::max_combine(gml::vec2(inner_vertices[ia].sv_position), gml::vec2(inner_vertices[ib].sv_position));
		max = gml::max_combine(max, gml::vec2(inner_vertices[ic].sv_position));

		int xmini = static_cast<int>((min.x * 0.5f + 0.5f) * m_width);
		int xmaxi = static_cast<int>((max.x * 0.5f + 0.5f) * m_width)+1;
		int ymini = static_cast<int>((min.y * 0.5f + 0.5f) * m_height);
		int ymaxi = static_cast<int>((max.y * 0.5f + 0.5f) * m_height)+1;
		if (xmini < 0) xmini = 0;
		if (ymini < 0) ymini = 0;
		if (xmaxi > m_width)xmaxi = m_width;
		if (ymaxi > m_height)ymaxi = m_height;

		gml::vec2 a = gml::vec2(inner_vertices[ia].sv_position);
		gml::vec2 ab = gml::vec2(inner_vertices[ib].sv_position) - a;
		gml::vec2 ac = gml::vec2(inner_vertices[ic].sv_position) - a;
		float det = gml::det22_t(ab, ac);
		if (gml::fequal(det ,0.0f))
		{
			continue;
		}
		
		//duel-face.
		float inv_det = 1.0f / det;
		//float inv_det = det < 0 ? 1.0f / det : -1.0f / det;

		for (int y = ymini; y < ymaxi; y++)
		{
			for (int x = xmini; x < xmaxi; x++)
			{
				//重心坐标算uv
				gml::vec2 pa = gml::vec2(x*2.0f / m_width - 1.0f, y * 2.0f / m_height - 1.0f) - a;
				float u = gml::det22_t(pa, ac) * inv_det;
				float v = gml::det22_t(ab, pa) * inv_det;
				if (u <0.0f || v < 0.0f || u > 1.0f || u + v >1.0f)
				{
					continue;
				}
				//这里buffer的y要注意
				int index = fragments.size();
				fragments.resize(index + 1);
				Fragment& f = fragments[index];
				f.x = x;
				f.y = m_height - y - 1;
				f.z = inner_vertices[ia].sv_position.z * (1.0f - u - v) +
					inner_vertices[ib].sv_position.z * u +
					inner_vertices[ic].sv_position.z * v;

				f.color = inner_vertices[ia].color * (1.0f - u - v) +
					inner_vertices[ib].color * u +
					inner_vertices[ic].color * v;
			}
		}
	}
}

void Renderer::PixelShader()
{
	int fragment_count = fragments.size();
	out_fragments.resize(fragment_count);
	for (int i = 0; i < fragment_count; i++)
	{
		Fragment& f = fragments[i];
		Fragment& of = out_fragments[i];
		
		of.x = f.x;
		of.y = f.y;
		of.z = f.z;
		of.color = f.color;
	}
}
void Renderer::OutputMerge() 
{
	int fragment_count = out_fragments.size();
	for (int i = 0; i < fragment_count; i++)
	{
		Fragment& f = out_fragments[i];
		int index = f.y * m_width + f.x;

		///z-test
		if (f.z >= 0.0f && f.z < m_depth_buffer[index])
		{
			m_depth_buffer[index] = f.z;
			m_color_buffer[index] = f.color;
		}
	}
}

void Renderer::CopyBuffer(byte* buffer, int width, int height, int pitch)
{
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			int index = pitch * h + w * 3;
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