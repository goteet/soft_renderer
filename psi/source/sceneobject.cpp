#include "pch.h"
#include <math.h>
#include "sceneobject.h"
#include <limits>
#include <gml/include/ray.h>

ISceneObject* ISceneObject::CreateSphere(const gml::vec3& position, float radius)
{
	return new SphereSceneObject(position, radius);
}
ISceneObject* ISceneObject::CreatePlane(const gml::vec3& position, const gml::vec3& normal)
{
	return new PlaneSceneObject(position, normal);
}
ISceneObject* ISceneObject::CreateBox(const gml::vec3& position, const gml::vec3& extends)
{
	return new BoxSceneObject(position, extends);
}
ISceneObject* ISceneObject::CreatePyramid(const gml::vec3& position, float extends)
{
	return new PyramidSceneObject(position, extends);
}
ISceneObject* ISceneObject::CreateModel(const gml::vec3& position, float size)
{
	return new ModelSceneObject(position, size);
}


ISceneObject::~ISceneObject()
{

}

void ISceneObject::Release()
{
	delete this;
}

SceneObject::SceneObject()
{

}

Material* SceneObject::GetMaterial()
{
	return const_cast<Material*>(const_cast<const SceneObject*>(this)->GetMaterial());

}

const Material* SceneObject::GetMaterial() const
{
	return &mMaterial;
}

const gml::vec3& SphereSceneObject::GetPosition() const
{
	return mSphere.GetCenter();
}

//////////////////////////////////////////////
//
SphereSceneObject::SphereSceneObject(const gml::vec3& position, float radius) :mSphere(position, radius)
{
	gml::vec3 r = gml::vec3(radius, radius, radius);
	mAABB.expand(position + r);
	mAABB.expand(position - r);
}

void SphereSceneObject::SetPosition(float x, float y, float z)
{
	mSphere.SetCenter(x, y, z);
}

void SphereSceneObject::SetPosition(const gml::vec3& center)
{
	mSphere.SetCenter(center);
}

bool SphereSceneObject::IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const
{
	float t0, t1;
	if (Intersect(ray, mSphere, t0, t1) > 0 && t0 < mint)
	{
		info.t = t0;
		info.normal = (ray.get_pos_by_len(t0) - mSphere.GetCenter()).normalize();
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////////////
//

PlaneSceneObject::PlaneSceneObject(const gml::vec3& position, const gml::vec3& normal) : mPlane(position, normal)
{
	gml::vec3 r = gml::vec3(
		std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::infinity());

	mAABB.expand(r);
	mAABB.expand(-r);
}

bool PlaneSceneObject::IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const
{
	float t;
	if (Intersect(ray, mPlane, t) > 0 && t < mint)
	{
		info.t = t;
		info.normal = mPlane.GetNormal();
		return true;
	}
	return false;
}

void PlaneSceneObject::SetPosition(float x, float y, float z)
{
	mPlane.SetPosition(x, y, z);
}

void PlaneSceneObject::SetPosition(const gml::vec3& center)
{
	mPlane.SetPosition(center);
}

const gml::vec3& PlaneSceneObject::GetPosition() const
{
	return mPlane.GetPosition();
}
//////////////////////////////////////////////
//

BoxSceneObject::BoxSceneObject(const gml::vec3& position, const gml::vec3& extend) : mBox(position, extend)
{
	mAABB.expand(position + mBox.GetAxisX() * extend.x);
	mAABB.expand(position - mBox.GetAxisX() * extend.x);
	mAABB.expand(position + mBox.GetAxisY() * extend.y);
	mAABB.expand(position - mBox.GetAxisY() * extend.y);
	mAABB.expand(position + mBox.GetAxisZ() * extend.z);
	mAABB.expand(position - mBox.GetAxisZ() * extend.z);
}

bool BoxSceneObject::IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const
{
	float t0, t1;
	if (Intersect(ray, mBox, t0, t1) > 0 && t0 < mint)
	{
		info.t = t0;

		gml::vec3 iNormal = (ray.get_pos_by_len(info.t) - mBox.GetCenter()).normalize();
		float absX = fabs(dot(iNormal, mBox.GetAxisX())) / mBox.GetExtend().x;
		float absY = fabs(dot(iNormal, mBox.GetAxisY())) / mBox.GetExtend().y;
		float absZ = fabs(dot(iNormal, mBox.GetAxisZ())) / mBox.GetExtend().z;
		if (absX > absY)
		{
			if (absX > absZ)
			{
				info.normal = iNormal.x > 0 ? mBox.GetAxisX() : -mBox.GetAxisX();
			}
			else
			{
				info.normal = iNormal.z > 0 ? mBox.GetAxisZ() : -mBox.GetAxisZ();
			}
		}
		else
		{
			if (absY > absZ)
			{
				info.normal = iNormal.y > 0 ? mBox.GetAxisY() : -mBox.GetAxisY();
			}
			else
			{
				info.normal = iNormal.z > 0 ? mBox.GetAxisZ() : -mBox.GetAxisZ();
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

void BoxSceneObject::SetPosition(float x, float y, float z)
{
	mBox.SetCenter(x, y, z);
}

void BoxSceneObject::SetPosition(const gml::vec3& center)
{
	mBox.SetCenter(center);
}

const gml::vec3& BoxSceneObject::GetPosition() const
{
	return mBox.GetCenter();
}

const gml::vec3 PyramidSceneObject::VERTS[4] = { gml::vec3(0, 0, 1), gml::vec3(1, 0, -0.5f), gml::vec3(-0.5f, -0.866f, -0.5f), gml::vec3(-0.5f, 0.866f, -0.5f) };
const int PyramidSceneObject::FACE[12] = {
	0, 1, 2,
	0, 2, 3,
	0, 3, 1,
	1, 2, 3,
};

PyramidSceneObject::PyramidSceneObject(const gml::vec3& position, float extend) :mCenter(position)
{
	for (int i = 0; i < 4; i++)
	{
		mVerts[i] = VERTS[i] * extend;
		mAABB.expand(mVerts[i]);
	}
}

bool PyramidSceneObject::IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const
{
	bool found = false;
	float t0, u, v;
	gml::vec3 normal;
	for (int i = 0; i < 12; i += 3)
	{
		if (IntersectTriangleWithRay(ray,
			mCenter + mVerts[FACE[i]],
			mCenter + mVerts[FACE[i + 1]],
			mCenter + mVerts[FACE[i + 2]],
			t0, u, v, normal))
		{
			if (found)
			{
				if (info.t > t0)
				{
					info.t = t0;
					info.normal = normal;
				}
			}
			else if (t0 < mint)
			{
				found = true;
				info.t = t0;
				info.normal = normal;
			}
		}
	}

	return  found;
}

void PyramidSceneObject::SetPosition(float x, float y, float z)
{
	mCenter.set(x, y, z);
}
void PyramidSceneObject::SetPosition(const gml::vec3& center)
{
	mCenter = center;
}

const gml::vec3& PyramidSceneObject::GetPosition() const
{
	return mCenter;
}

const float TEAPOT_VERTS[] = {
	0.70f, 1.20f, -0.00f, 0.70f, 1.25f, -0.00f, 0.75f, 1.20f, -0.00f, 0.50f, 1.20f, 0.50f, 0.50f, 1.25f, 0.50f, 0.53f, 1.20f, 0.53f, 0.00f, 1.20f, 0.70f, 0.00f, 1.25f, 0.70f, 0.00f, 1.20f, 0.75f, -0.51f, 1.20f, 0.50f, -0.50f, 1.25f, 0.50f, -0.53f, 1.20f, 0.53f, -0.70f, 1.20f, -0.00f, -0.70f, 1.25f, -0.00f, -0.75f, 1.20f, -0.00f, -0.50f, 1.20f, -0.50f, -0.50f, 1.25f, -0.50f, -0.53f, 1.20f, -0.53f, 0.00f, 1.20f, -0.70f, 0.00f, 1.25f, -0.70f, 0.00f, 1.20f, -0.75f, 0.50f, 1.20f, -0.50f, 0.50f, 1.25f, -0.50f, 0.53f, 1.20f, -0.53f, 0.92f, 0.81f, -0.00f, 1.00f, 0.45f, -0.00f, 0.65f, 0.81f, 0.65f, 0.71f, 0.45f, 0.71f, 0.00f, 0.81f, 0.92f, 0.00f, 0.45f, 1.00f, -0.65f, 0.81f, 0.65f, -0.71f, 0.45f, 0.71f, -0.92f, 0.81f, -0.00f, -1.00f, 0.45f, -0.00f, -0.65f, 0.81f, -0.65f, -0.71f, 0.45f, -0.71f, 0.00f, 0.81f, -0.92f, 0.00f, 0.45f, -1.00f, 0.65f, 0.81f, -0.65f, 0.71f, 0.45f, -0.71f, 0.88f, 0.19f, -0.00f, 0.75f, 0.08f, -0.00f, 0.62f, 0.19f, 0.62f, 0.53f, 0.08f, 0.53f, 0.00f, 0.19f, 0.88f, 0.00f, 0.08f, 0.75f, -0.62f, 0.19f, 0.62f, -0.53f, 0.08f, 0.53f, -0.88f, 0.19f, -0.00f, -0.75f, 0.08f, -0.00f, -0.62f, 0.19f, -0.62f, -0.53f, 0.08f, -0.53f, 0.00f, 0.19f, -0.88f, 0.00f, 0.08f, -0.75f, 0.62f, 0.19f, -0.62f, 0.53f, 0.08f, -0.53f, 0.64f, 0.02f, -0.00f, 0.00f, 0.00f, -0.00f, 0.46f, 0.02f, 0.46f, 0.00f, 0.02f, 0.64f, -0.46f, 0.02f, 0.46f, -0.64f, 0.02f, -0.00f, -0.46f, 0.02f, -0.46f, 0.00f, 0.02f, -0.64f, 0.46f, 0.02f, -0.46f, -0.80f, 1.01f, -0.00f, -1.21f, 1.00f, -0.00f, -1.35f, 0.90f, -0.00f, -0.77f, 1.07f, 0.11f, -1.26f, 1.05f, 0.11f, -1.42f, 0.90f, 0.11f, -0.75f, 1.13f, -0.00f, -1.31f, 1.10f, -0.00f, -1.50f, 0.90f, -0.00f, -0.78f, 1.07f, -0.11f, -1.26f, 1.05f, -0.11f, -1.43f, 0.90f, -0.11f, -1.27f, 0.68f, -0.00f, -1.00f, 0.45f, -0.00f, -1.32f, 0.63f, 0.11f, -0.97f, 0.38f, 0.11f, -1.37f, 0.58f, -0.00f, -0.95f, 0.30f, -0.00f, -1.32f, 0.63f, -0.11f, -0.98f, 0.38f, -0.11f, 0.85f, 0.71f, -0.00f, 1.19f, 0.90f, -0.00f, 1.35f, 1.20f, -0.00f, 0.85f, 0.51f, 0.25f, 1.27f, 0.81f, 0.17f, 1.50f, 1.20f, 0.09f, 0.85f, 0.30f, -0.00f, 1.34f, 0.72f, -0.00f, 1.65f, 1.20f, -0.00f, 0.85f, 0.51f, -0.25f, 1.27f, 0.81f, -0.17f, 1.50f, 1.20f, -0.09f, 1.41f, 1.23f, -0.00f, 1.40f, 1.20f, -0.00f, 1.56f, 1.23f, 0.07f, 1.50f, 1.20f, 0.06f, 1.71f, 1.24f, -0.00f, 1.60f, 1.20f, -0.00f, 1.56f, 1.23f, -0.07f, 1.50f, 1.20f, -0.06f, 0.00f, 1.58f, -0.00f, 0.16f, 1.49f, -0.00f, 0.10f, 1.35f, -0.00f, 0.12f, 1.49f, 0.12f, 0.07f, 1.35f, 0.07f, 0.00f, 1.49f, 0.16f, 0.00f, 1.35f, 0.10f, -0.12f, 1.49f, 0.12f, -0.07f, 1.35f, 0.07f, -0.16f, 1.49f, -0.00f, -0.10f, 1.35f, -0.00f, -0.12f, 1.49f, -0.12f, -0.07f, 1.35f, -0.07f, 0.00f, 1.49f, -0.16f, 0.00f, 1.35f, -0.10f, 0.12f, 1.49f, -0.12f, 0.07f, 1.35f, -0.07f, 0.41f, 1.27f, -0.00f, 0.65f, 1.20f, -0.00f, 0.29f, 1.27f, 0.29f, 0.46f, 1.20f, 0.46f, 0.00f, 1.27f, 0.41f, 0.00f, 1.20f, 0.65f, -0.29f, 1.27f, 0.29f, -0.46f, 1.20f, 0.46f, -0.41f, 1.27f, -0.00f, -0.65f, 1.20f, -0.00f, -0.29f, 1.27f, -0.29f, -0.46f, 1.20f, -0.46f, 0.00f, 1.27f, -0.41f, 0.00f, 1.20f, -0.65f, 0.29f, 1.27f, -0.29f, 0.46f, 1.20f, -0.46f,
};

const int  TEAPOT_INDEX[] = { 1 - 1, 4 - 1, 5 - 1, 5 - 1, 2 - 1, 1 - 1, 2 - 1, 5 - 1, 6 - 1, 6 - 1, 3 - 1, 2 - 1, 4 - 1, 7 - 1, 8 - 1, 8 - 1, 5 - 1, 4 - 1, 5 - 1, 8 - 1, 9 - 1, 9 - 1, 6 - 1, 5 - 1, 7 - 1, 10 - 1, 11 - 1, 11 - 1, 8 - 1, 7 - 1, 8 - 1, 11 - 1, 12 - 1, 12 - 1, 9 - 1, 8 - 1, 10 - 1, 13 - 1, 14 - 1, 14 - 1, 11 - 1, 10 - 1, 11 - 1, 14 - 1, 15 - 1, 15 - 1, 12 - 1, 11 - 1, 13 - 1, 16 - 1, 17 - 1, 17 - 1, 14 - 1, 13 - 1, 14 - 1, 17 - 1, 18 - 1, 18 - 1, 15 - 1, 14 - 1, 16 - 1, 19 - 1, 20 - 1, 20 - 1, 17 - 1, 16 - 1, 17 - 1, 20 - 1, 21 - 1, 21 - 1, 18 - 1, 17 - 1, 19 - 1, 22 - 1, 23 - 1, 23 - 1, 20 - 1, 19 - 1, 20 - 1, 23 - 1, 24 - 1, 24 - 1, 21 - 1, 20 - 1, 22 - 1, 1 - 1, 2 - 1, 2 - 1, 23 - 1, 22 - 1, 23 - 1, 2 - 1, 3 - 1, 3 - 1, 24 - 1, 23 - 1, 3 - 1, 6 - 1, 27 - 1, 27 - 1, 25 - 1, 3 - 1, 25 - 1, 27 - 1, 28 - 1, 28 - 1, 26 - 1, 25 - 1, 6 - 1, 9 - 1, 29 - 1, 29 - 1, 27 - 1, 6 - 1, 27 - 1, 29 - 1, 30 - 1, 30 - 1, 28 - 1, 27 - 1, 9 - 1, 12 - 1, 31 - 1, 31 - 1, 29 - 1, 9 - 1, 29 - 1, 31 - 1, 32 - 1, 32 - 1, 30 - 1, 29 - 1, 12 - 1, 15 - 1, 33 - 1, 33 - 1, 31 - 1, 12 - 1, 31 - 1, 33 - 1, 34 - 1, 34 - 1, 32 - 1, 31 - 1, 15 - 1, 18 - 1, 35 - 1, 35 - 1, 33 - 1, 15 - 1, 33 - 1, 35 - 1, 36 - 1, 36 - 1, 34 - 1, 33 - 1, 18 - 1, 21 - 1, 37 - 1, 37 - 1, 35 - 1, 18 - 1, 35 - 1, 37 - 1, 38 - 1, 38 - 1, 36 - 1, 35 - 1, 21 - 1, 24 - 1, 39 - 1, 39 - 1, 37 - 1, 21 - 1, 37 - 1, 39 - 1, 40 - 1, 40 - 1, 38 - 1, 37 - 1, 24 - 1, 3 - 1, 25 - 1, 25 - 1, 39 - 1, 24 - 1, 39 - 1, 25 - 1, 26 - 1, 26 - 1, 40 - 1, 39 - 1, 26 - 1, 28 - 1, 43 - 1, 43 - 1, 41 - 1, 26 - 1, 41 - 1, 43 - 1, 44 - 1, 44 - 1, 42 - 1, 41 - 1, 28 - 1, 30 - 1, 45 - 1, 45 - 1, 43 - 1, 28 - 1, 43 - 1, 45 - 1, 46 - 1, 46 - 1, 44 - 1, 43 - 1, 30 - 1, 32 - 1, 47 - 1, 47 - 1, 45 - 1, 30 - 1, 45 - 1, 47 - 1, 48 - 1, 48 - 1, 46 - 1, 45 - 1, 32 - 1, 34 - 1, 49 - 1, 49 - 1, 47 - 1, 32 - 1, 47 - 1, 49 - 1, 50 - 1, 50 - 1, 48 - 1, 47 - 1, 34 - 1, 36 - 1, 51 - 1, 51 - 1, 49 - 1, 34 - 1, 49 - 1, 51 - 1, 52 - 1, 52 - 1, 50 - 1, 49 - 1, 36 - 1, 38 - 1, 53 - 1, 53 - 1, 51 - 1, 36 - 1, 51 - 1, 53 - 1, 54 - 1, 54 - 1, 52 - 1, 51 - 1, 38 - 1, 40 - 1, 55 - 1, 55 - 1, 53 - 1, 38 - 1, 53 - 1, 55 - 1, 56 - 1, 56 - 1, 54 - 1, 53 - 1, 40 - 1, 26 - 1, 41 - 1, 41 - 1, 55 - 1, 40 - 1, 55 - 1, 41 - 1, 42 - 1, 42 - 1, 56 - 1, 55 - 1, 42 - 1, 44 - 1, 59 - 1, 59 - 1, 57 - 1, 42 - 1, 57 - 1, 59 - 1, 58 - 1, 58 - 1, 58 - 1, 57 - 1, 44 - 1, 46 - 1, 60 - 1, 60 - 1, 59 - 1, 44 - 1, 59 - 1, 60 - 1, 58 - 1, 58 - 1, 58 - 1, 59 - 1, 46 - 1, 48 - 1, 61 - 1, 61 - 1, 60 - 1, 46 - 1, 60 - 1, 61 - 1, 58 - 1, 58 - 1, 58 - 1, 60 - 1, 48 - 1, 50 - 1, 62 - 1, 62 - 1, 61 - 1, 48 - 1, 61 - 1, 62 - 1, 58 - 1, 58 - 1, 58 - 1, 61 - 1, 50 - 1, 52 - 1, 63 - 1, 63 - 1, 62 - 1, 50 - 1, 62 - 1, 63 - 1, 58 - 1, 58 - 1, 58 - 1, 62 - 1, 52 - 1, 54 - 1, 64 - 1, 64 - 1, 63 - 1, 52 - 1, 63 - 1, 64 - 1, 58 - 1, 58 - 1, 58 - 1, 63 - 1, 54 - 1, 56 - 1, 65 - 1, 65 - 1, 64 - 1, 54 - 1, 64 - 1, 65 - 1, 58 - 1, 58 - 1, 58 - 1, 64 - 1, 56 - 1, 42 - 1, 57 - 1, 57 - 1, 65 - 1, 56 - 1, 65 - 1, 57 - 1, 58 - 1, 58 - 1, 58 - 1, 65 - 1, 66 - 1, 69 - 1, 70 - 1, 70 - 1, 67 - 1, 66 - 1, 67 - 1, 70 - 1, 71 - 1, 71 - 1, 68 - 1, 67 - 1, 69 - 1, 72 - 1, 73 - 1, 73 - 1, 70 - 1, 69 - 1, 70 - 1, 73 - 1, 74 - 1, 74 - 1, 71 - 1, 70 - 1, 72 - 1, 75 - 1, 76 - 1, 76 - 1, 73 - 1, 72 - 1, 73 - 1, 76 - 1, 77 - 1, 77 - 1, 74 - 1, 73 - 1, 75 - 1, 66 - 1, 67 - 1, 67 - 1, 76 - 1, 75 - 1, 76 - 1, 67 - 1, 68 - 1, 68 - 1, 77 - 1, 76 - 1, 68 - 1, 71 - 1, 80 - 1, 80 - 1, 78 - 1, 68 - 1, 78 - 1, 80 - 1, 81 - 1, 81 - 1, 79 - 1, 78 - 1, 71 - 1, 74 - 1, 82 - 1, 82 - 1, 80 - 1, 71 - 1, 80 - 1, 82 - 1, 83 - 1, 83 - 1, 81 - 1, 80 - 1, 74 - 1, 77 - 1, 84 - 1, 84 - 1, 82 - 1, 74 - 1, 82 - 1, 84 - 1, 85 - 1, 85 - 1, 83 - 1, 82 - 1, 77 - 1, 68 - 1, 78 - 1, 78 - 1, 84 - 1, 77 - 1, 84 - 1, 78 - 1, 79 - 1, 79 - 1, 85 - 1, 84 - 1, 86 - 1, 89 - 1, 90 - 1, 90 - 1, 87 - 1, 86 - 1, 87 - 1, 90 - 1, 91 - 1, 91 - 1, 88 - 1, 87 - 1, 89 - 1, 92 - 1, 93 - 1, 93 - 1, 90 - 1, 89 - 1, 90 - 1, 93 - 1, 94 - 1, 94 - 1, 91 - 1, 90 - 1, 92 - 1, 95 - 1, 96 - 1, 96 - 1, 93 - 1, 92 - 1, 93 - 1, 96 - 1, 97 - 1, 97 - 1, 94 - 1, 93 - 1, 95 - 1, 86 - 1, 87 - 1, 87 - 1, 96 - 1, 95 - 1, 96 - 1, 87 - 1, 88 - 1, 88 - 1, 97 - 1, 96 - 1, 88 - 1, 91 - 1, 100 - 1, 100 - 1, 98 - 1, 88 - 1, 98 - 1, 100 - 1, 101 - 1, 101 - 1, 99 - 1, 98 - 1, 91 - 1, 94 - 1, 102 - 1, 102 - 1, 100 - 1, 91 - 1, 100 - 1, 102 - 1, 103 - 1, 103 - 1, 101 - 1, 100 - 1, 94 - 1, 97 - 1, 104 - 1, 104 - 1, 102 - 1, 94 - 1, 102 - 1, 104 - 1, 105 - 1, 105 - 1, 103 - 1, 102 - 1, 97 - 1, 88 - 1, 98 - 1, 98 - 1, 104 - 1, 97 - 1, 104 - 1, 98 - 1, 99 - 1, 99 - 1, 105 - 1, 104 - 1, 106 - 1, 106 - 1, 109 - 1, 109 - 1, 107 - 1, 106 - 1, 107 - 1, 109 - 1, 110 - 1, 110 - 1, 108 - 1, 107 - 1, 106 - 1, 106 - 1, 111 - 1, 111 - 1, 109 - 1, 106 - 1, 109 - 1, 111 - 1, 112 - 1, 112 - 1, 110 - 1, 109 - 1, 106 - 1, 106 - 1, 113 - 1, 113 - 1, 111 - 1, 106 - 1, 111 - 1, 113 - 1, 114 - 1, 114 - 1, 112 - 1, 111 - 1, 106 - 1, 106 - 1, 115 - 1, 115 - 1, 113 - 1, 106 - 1, 113 - 1, 115 - 1, 116 - 1, 116 - 1, 114 - 1, 113 - 1, 106 - 1, 106 - 1, 117 - 1, 117 - 1, 115 - 1, 106 - 1, 115 - 1, 117 - 1, 118 - 1, 118 - 1, 116 - 1, 115 - 1, 106 - 1, 106 - 1, 119 - 1, 119 - 1, 117 - 1, 106 - 1, 117 - 1, 119 - 1, 120 - 1, 120 - 1, 118 - 1, 117 - 1, 106 - 1, 106 - 1, 121 - 1, 121 - 1, 119 - 1, 106 - 1, 119 - 1, 121 - 1, 122 - 1, 122 - 1, 120 - 1, 119 - 1, 106 - 1, 106 - 1, 107 - 1, 107 - 1, 121 - 1, 106 - 1, 121 - 1, 107 - 1, 108 - 1, 108 - 1, 122 - 1, 121 - 1, 108 - 1, 110 - 1, 125 - 1, 125 - 1, 123 - 1, 108 - 1, 123 - 1, 125 - 1, 126 - 1, 126 - 1, 124 - 1, 123 - 1, 110 - 1, 112 - 1, 127 - 1, 127 - 1, 125 - 1, 110 - 1, 125 - 1, 127 - 1, 128 - 1, 128 - 1, 126 - 1, 125 - 1, 112 - 1, 114 - 1, 129 - 1, 129 - 1, 127 - 1, 112 - 1, 127 - 1, 129 - 1, 130 - 1, 130 - 1, 128 - 1, 127 - 1, 114 - 1, 116 - 1, 131 - 1, 131 - 1, 129 - 1, 114 - 1, 129 - 1, 131 - 1, 132 - 1, 132 - 1, 130 - 1, 129 - 1, 116 - 1, 118 - 1, 133 - 1, 133 - 1, 131 - 1, 116 - 1, 131 - 1, 133 - 1, 134 - 1, 134 - 1, 132 - 1, 131 - 1, 118 - 1, 120 - 1, 135 - 1, 135 - 1, 133 - 1, 118 - 1, 133 - 1, 135 - 1, 136 - 1, 136 - 1, 134 - 1, 133 - 1, 120 - 1, 122 - 1, 137 - 1, 137 - 1, 135 - 1, 120 - 1, 135 - 1, 137 - 1, 138 - 1, 138 - 1, 136 - 1, 135 - 1, 122 - 1, 108 - 1, 123 - 1, 123 - 1, 137 - 1, };

const int TEAPOT_INDEX_COUNT = sizeof(TEAPOT_INDEX) / sizeof(int);
const int TEAPOT_VERT_COUNT = 138;

ModelSceneObject::ModelSceneObject(const gml::vec3& position, float size) :mCenter(position)
{

	for (int i = 0; i < TEAPOT_VERT_COUNT; i++)
	{
		mVerts[i].set(TEAPOT_VERTS[i * 3], TEAPOT_VERTS[i * 3 + 1], TEAPOT_VERTS[i * 3 + 2]) *= size;
		mAABB.expand(mVerts[i]);
	}
}

bool ModelSceneObject::IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const
{
	bool found = false;
	float t0, u, v;
	gml::vec3 normal;
	for (int i = 0; i < TEAPOT_INDEX_COUNT; i += 3)
	{
		if (IntersectTriangleWithRay(ray,
			mCenter + mVerts[TEAPOT_INDEX[i]],
			mCenter + mVerts[TEAPOT_INDEX[i + 1]],
			mCenter + mVerts[TEAPOT_INDEX[i + 2]],
			t0, u, v, normal))
		{
			if (found)
			{
				if (info.t > t0)
				{
					info.t = t0;
					info.normal = normal;
				}
			}
			else if (t0 < mint)
			{
				found = true;
				info.t = t0;
				info.normal = normal;
			}
		}
	}
	return found;
}

void ModelSceneObject::SetPosition(float x, float y, float z)
{
	mCenter.set(x, y, z);
}
void ModelSceneObject::SetPosition(const gml::vec3& center)
{
	mCenter = center;
}

const gml::vec3& ModelSceneObject::GetPosition() const
{
	return mCenter;
}
