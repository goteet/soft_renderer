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

	//��ʱ����ǿ�ȣ����ջ�û��ʼд��
	float Intensity = 1.0f;
};