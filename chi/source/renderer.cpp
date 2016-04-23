#include "pch.h"
#include "renderer.h"
#include <gml/include/math_util.h>
#include <gml/include/matrix.h>
#include <vector>

struct Fragment
{
	int x;
	int y;
	int stencil = -1;
	float z;
	gml::vec2 uv;
	gml::color4 color;
	
};

namespace
{
	const float FOV = static_cast<float>(65.0 / 180.0f * gml::PI);
	const float size = 1.00f;
	Vertex vertices[8] =
	{
		{ { -size, -size, 0.0f, 1.0f } ,{}, { 0.0f, 0.0f } },
		{ { -size, +size, 0.0f, 1.0f } ,{}, { 0.0f, 1.0f } },
		{ { +size, +size, 0.0f, 1.0f } ,{}, { 1.0f, 1.0f } },
		{ { +size, -size, 0.0f, 1.0f } ,{}, { 0.0f, 1.0f } },
		{ { 0.0f, -size, -size, 1.0f } ,{}, { 0.0f, 0.0f } },
		{ { 0.0f, +size, -size, 1.0f } ,{}, { 1.0f, 0.0f }},
		{ { 0.0f, +size, +size, 1.0f } ,{}, { 1.0f, 1.0f } },
		{ { 0.0f, -size, +size, 1.0f } ,{}, { 0.0f, 1.0f } },
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
		gml::vec2 texcoord;
	};

	std::vector<V2F> inner_vertices;
	std::vector<Fragment> fragments;
	std::vector<Fragment> out_fragments;
	

	gml::color4 SampleGridTexture(gml::vec2 uv)
	{
		const int SCALER = 8;
		int u = uv.x * SCALER;
		int v = uv.y * SCALER;
		u %= 2;
		v %= 2;
		return ((u^v) ? gml::color4::white : gml::color4::gray);
	}
}



Renderer::Renderer(int width, int height)
{
	m_width = width;
	m_height = height;

	m_color_buffer = new gml::color4[width * height];
	m_depth_buffer = new float[width * height];
	m_stencil_buffer = new byte[width * height];

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

	const float ALPHA = 0.5f;
	vertices[0].color.a = 0;
	vertices[1].color.a = ALPHA;
	vertices[2].color.a = ALPHA;
	vertices[3].color.a = ALPHA;
	vertices[4].color.a = ALPHA;
	vertices[5].color.a = ALPHA;
	vertices[6].color.a = ALPHA;
	vertices[7].color.a = 0;

	m_mat_world = gml::mat44::scale(1.5f,1.5f,1.5f);
	m_mat_view = gml::mat44::look_at(gml::vec3(0.0f, -2.0f, -5.0f), gml::vec3::zero, gml::vec3(0, 1, 0));
	m_mat_proj = gml::mat44::perspective(FOV, m_width * 1.0f / m_height, 1.0f, 1000.0f);
	
	m_mat_mv = m_mat_view * m_mat_world;
	m_mat_vp = m_mat_proj * m_mat_view;
	m_mat_mvp = m_mat_proj * m_mat_mv;

	m_using_scissor = true;

	m_clear_color = gml::color4::black;
	m_clear_depth = 1.0f;
	m_clear_stencil = 0;
}

Renderer::~Renderer()
{
	delete[] m_color_buffer;
	delete[] m_depth_buffer;
}

void Renderer::Render()
{
	static float r = 0.0f;
	r += 0.01f;
	const float radius = 5.0f;

	m_mat_view = gml::mat44::look_at(gml::vec3(sin(r)*radius, 5.0f, cos(r)*radius), gml::vec3::zero, gml::vec3(0, 1, 0));

	m_mat_mv = m_mat_view * m_mat_world;
	m_mat_vp = m_mat_proj * m_mat_view;
	m_mat_mvp = m_mat_proj * m_mat_mv;

	if(m_using_scissor) PushScissorRect();
	ClearBuffer();
	VertexShader();
	Rasterization();
	PixelShader();
	OutputMerge();
	if (m_using_scissor) PopScissorRect();
}

void Renderer::ClearBuffer()
{
	for (int h = 0; h < m_height; h++)
	{
		for (int w = 0; w < m_width; w++)
		{
			int index = h * m_width + w;
			m_clear_color.r = h * 0.2f / m_height;
			m_color_buffer[index] = m_clear_color;
			m_depth_buffer[index] = m_clear_depth;
			m_stencil_buffer[index] = m_clear_stencil;
		}
	}

	fragments.clear();
}
void Renderer::VertexShader() 
{
	inner_vertices.resize(m_vertex_count);
	for (int i = 0; i < m_vertex_count; i++)
	{
		Vertex& inp = vertices[i];
		V2F& out = inner_vertices[i];
		out.sv_position = m_mat_mvp * inp.position;
		float inv_z = 1.0f / out.sv_position.w;
		out.sv_position.x *= inv_z;
		out.sv_position.y *= inv_z;
		out.sv_position.z *= inv_z;
		out.color = inp.color;
		out.texcoord = inp.texcoord;

	}
}

void Renderer::Rasterization()
{
	for (int triangle = 0; triangle < m_triangle_count; triangle++)
	{
		Index& ia = indices[triangle * 3 + 0];
		Index& ib = indices[triangle * 3 + 1];
		Index& ic = indices[triangle * 3 + 2];
		V2F& pa = inner_vertices[ia];
		V2F& pb = inner_vertices[ib];
		V2F& pc = inner_vertices[ic];

		gml::vec2 min = gml::min_combine(gml::vec2(pa.sv_position), gml::vec2(pb.sv_position));
		min = gml::min_combine(min, gml::vec2(pc.sv_position));

		gml::vec2 max = gml::max_combine(gml::vec2(pa.sv_position), gml::vec2(pb.sv_position));
		max = gml::max_combine(max, gml::vec2(pc.sv_position));

		int xmini = static_cast<int>((min.x * 0.5f + 0.5f) * m_width);
		int xmaxi = static_cast<int>((max.x * 0.5f + 0.5f) * m_width)+1;
		int ymini = static_cast<int>((min.y * 0.5f + 0.5f) * m_height);
		int ymaxi = static_cast<int>((max.y * 0.5f + 0.5f) * m_height)+1;
		if (xmini < 0) xmini = 0;
		if (ymini < 0) ymini = 0;
		if (xmaxi > m_width)xmaxi = m_width;
		if (ymaxi > m_height)ymaxi = m_height;

		gml::vec2 a = gml::vec2(pa.sv_position);
		gml::vec2 ab = gml::vec2(pb.sv_position) - a;
		gml::vec2 ac = gml::vec2(pc.sv_position) - a;
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
				gml::vec2 ta = gml::vec2(x*2.0f / m_width - 1.0f, y * 2.0f / m_height - 1.0f) - a;
				float u = gml::det22_t(ta, ac) * inv_det;
				float v = gml::det22_t(ab, ta) * inv_det;
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

				float w = 1.0f - u - v;
				f.z = pa.sv_position.z * w +
					pb.sv_position.z * u +
					pc.sv_position.z * v;

				f.color = pa.color * w +
					pb.color * u +
					pc.color * v;

				//矫正透视
				float inv_wa = 1.0f / pa.sv_position.w;
				float inv_wb = 1.0f / pb.sv_position.w;
				float inv_wc = 1.0f / pc.sv_position.w;
				f.uv = pa.texcoord * w * inv_wa + 
					pb.texcoord * u * inv_wb +
					pc.texcoord * v * inv_wc;

				//这里有更好的办法，过后整理vertex out再说
				float inv_w = inv_wa * w + inv_wb * u + inv_wc * v;
				f.uv *= 1.0f / inv_w;
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
		gml::color4 sample = SampleGridTexture(f.uv);
		of.color = f.color.clamped() * sample;
		of.color.clamp();
	}
}
void Renderer::OutputMerge() 
{
	int fragment_count = out_fragments.size();
	for (int i = 0; i < fragment_count; i++)
	{
		Fragment& f = out_fragments[i];
		int index = f.y * m_width + f.x;

		//alpha-test
		if (f.color.a < 0.001f)
		{
			continue; ///discard;
		}

		//scissor-test
		if (m_scissor_rects.size() != 0)
		{
			bool in_scissor_rect = false;
			for (auto& r : m_scissor_rects)
			{
				if (r.hit_test(f.x, f.y) != gml::HIT_TYPE::outside)
				{
					in_scissor_rect = true;
					break;
				}
			}
			if (!in_scissor_rect)
			{
				continue;
			}
		}

		//stencil-test
		if (f.stencil >= 0)
		{
			if (f.stencil > m_stencil_buffer[index])
			{
				m_stencil_buffer[index] = f.stencil;
			}
			else
			{
				continue; ///discard;
			}
		}

		// z-test
		if (f.z >= 0.0f && f.z < m_depth_buffer[index])
		{
			m_depth_buffer[index] = f.z;
		}
		else
		{
			continue; ///discard;
		}

		// blending
		auto& dst = m_color_buffer[index];
		auto& src = f.color;
		dst.replace(gml::lerp(gml::swizzle<gml::R, gml::G, gml::B>(dst), gml::swizzle<gml::R, gml::G, gml::B>(src), src.a));
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
			gml::color4& color = m_color_buffer[src_index].clamp();

			unsigned int color32 = color.to_rgba();
			buffer[index + 0] = (color32 >> 16) & 0xFF;
			buffer[index + 1] = (color32 >> 8) & 0xFF;
			buffer[index + 2] = color32 & 0xFF;
		}
	}
}

void Renderer::PushScissorRect()
{
	float left = -0.1f;
	float right = 0.1f;
	float top = 0.2f;
	float bottom = -0.2f;

	gml::rect r;
	left = left * 0.5f + 0.5f;
	right = right * 0.5f + 0.5f;
	top = 0.5f - top * 0.5f;
	bottom = 0.5f - bottom * 0.5f;

	r.set_left(left * m_width);
	r.set_right(right * m_width);
	r.set_top(top * m_height);
	r.set_bottom(bottom * m_height);
	m_scissor_rects.push_back(r);
}

void Renderer::PopScissorRect()
{
	if (m_scissor_rects.size() != 0)
	{
		m_scissor_rects.pop_back();
	}
}