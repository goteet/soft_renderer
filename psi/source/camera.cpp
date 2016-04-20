#include "pch.h"
#include <math.h>
#include "camera.h"

Camera::Camera()
{
	SetFOV(30);
}

gml::ray Camera::GenerateRay(int width, int height, int x, int y)
{
	float pixelX = x + 0.5f;	//加上半个像素的宽度
	float pixelY = y + 0.5f;	//加上半个像素的宽度

	float widthInv = 1.0f / width;
	float heightInv = 1.0f / height;

	float aspect = width * heightInv;
	
	float xReal = pixelX * widthInv * 2.0f - 1.0f;
	float yReal = pixelY * heightInv * 2.0f - 1.0f;

	gml::ray ray;
	ray.set_dir(gml::vec3(xReal * aspect * mTangentFOV, yReal  * mTangentFOV, -1).normalized());
	ray.set_origin(mPosition);
	return ray;
}

void Camera::SetPosition(float x, float y, float z)
{
	mPosition.set(x, y, z);
}

void Camera::SetFOV(float angle)
{
	if (angle >= 180.0f)
	{
		angle = 179.99f;
	}
	else if (angle < 10.0f)
	{
		angle = 10.0f;
	}

	mFOV = angle * 3.141592653f / 180.0f;
	mTangentFOV = tanf(mFOV * 0.5f);
}