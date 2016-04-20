#pragma once 
#include <gml/include/ray.h>

class Camera
{
public:
	Camera();

	void SetPosition(float x, float y, float z);

	void SetFOV(float angle);

	gml::ray GenerateRay(int w, int h, int x, int y);

	inline const gml::vec3& GetPosition() const { return mPosition; }

private:
	gml::vec3 mPosition;

	float mFOV;
	float mTangentFOV;

};