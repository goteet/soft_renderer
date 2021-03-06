#pragma once
#include "pch.h"
#include <gmlvector.h>
#include <gmlmatrix.h>
#include <gmlcolor.h>
#include <gmlrect.h>
#include <vector>

typedef byte Index;

struct Vertex
{
	gml::vec4 position;
	gml::color4 color;
	gml::vec2 texcoord;
};

class Renderer
{
public:
	Renderer(int width, int height);
	~Renderer();
	void Render();
	void CopyBuffer(byte* buffer, int width, int height, int pitch);

private:
	void ClearBuffer();
	void VertexShader();
	void Rasterization();
	void PixelShader();
	void OutputMerge();

	void PushScissorRect();
	void PopScissorRect();

	bool m_using_scissor = false;

	int m_width;
	int m_height;
	gml::color4 m_clear_color;
	gml::color4* m_color_buffer;
	byte	m_clear_stencil;
	byte*	m_stencil_buffer;
	float	m_clear_depth;
	float*  m_depth_buffer;

	gml::mat44 m_mat_world;
	gml::mat44 m_mat_view;
	gml::mat44 m_mat_proj;
	gml::mat44 m_mat_mvp;
	gml::mat44 m_mat_mv;
	gml::mat44 m_mat_vp;

	int m_vertex_count;
	int m_triangle_count;
	std::vector<gml::rect> m_scissor_rects;
};
