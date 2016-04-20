#include "pch.h"
#include "renderobject.h"

RenderObject* RenderObject::CreateSphere(float radius, float x, float y, float z, bool reflective, bool transparency)
{
	Sphere* sphere = new Sphere(x, y, z, radius);
	RenderObject* result = new RenderObject(sphere);

	result->mMaterial.IsReflective = reflective;
	result->mMaterial.IsTransparent = transparency;

	return result;
}

RenderObject::RenderObject(Sphere* s)
{
	mCollision = s;
}

RenderObject::~RenderObject()
{
	if (mCollision == nullptr)
	{
		delete mCollision;
		mCollision = nullptr;
	}
}