#pragma once

#include "material.h"
#include "mathx.h"
#include "icollision.h"

class RenderObject
{
public:
	static RenderObject* CreateSphere(float radius, float x, float y, float z, bool reflective, bool transparency);

	~RenderObject();

	inline const Material& GetMaterial() const	{ return mMaterial; }

	inline ICollision* GetCollision() const		{ return mCollision; }

private:
	RenderObject(Sphere* s);

	ICollision* mCollision = nullptr;

	Material mMaterial;

};