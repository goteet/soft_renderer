#pragma once
#include <gmlvector.h>
#include <gmlcolor.h>


class Material
{
public:
	bool IsReflective = false;
	bool IsTransparent = false;
};


class Light
{
public:
	gml::vec3 Position;
	gml::color3 Color;

	//临时给个强度，光照还没开始写。
	float Intensity = 1.0f;
};