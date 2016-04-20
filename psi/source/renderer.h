#pragma once
#include <vector>
#include <irenderer.h>
#include "camera.h"
#include "sceneobject.h"
#include <gml/include/color.h>

struct PresentStuff;

class Renderer : public IRenderer
{
public:
	virtual void Present(const IScene* scene, unsigned char* buffer, int width, int height, int pitch);

private:
	void InternalPresent(PresentStuff* seg, const IScene* scene);
	gml::color3 Trace(const IScene* scene, const gml::ray& ray, int reccursiveDepth);
	
	Camera  mCamera;

	gml::color3 mClearColor = gml::color3::black;
};