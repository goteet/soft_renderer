#include "pch.h"
#include "irenderer.h"
#include "renderer.h"

IRenderer* IRenderer::Create(int width, int height)
{
	return new Renderer(width, height);
}

IRenderer::~IRenderer()
{

}

void IRenderer::Release()
{
	delete this;
}