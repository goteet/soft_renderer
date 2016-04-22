#pragma once
#include "pch.h"
#include <gml/include/vector.h>
#include <gml/include/color.h>

typedef byte Index;

struct Vertex
{
	gml::vec4 position;
	gml::color4 color;
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

	int m_width;
	int m_height;
	gml::color4 m_clear_color;
	gml::color4* m_color_buffer;


	int m_triangle_count;
};
