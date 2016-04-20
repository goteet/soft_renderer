#pragma once
#include <isceneobject.h>

class Sphere
{
public:
	Sphere(const gml::vec3& center, float r);

	Sphere& SetCenter(float x, float y, float z);

	Sphere& SetCenter(const gml::vec3& center);

	Sphere& SetRadius(float r);

	inline const gml::vec3& GetCenter() const { return mCenter; }

	inline float GetRadius() const { return mRadius; }

	inline float GetRadiusSquare() const { return mRadiusSquare; }

private:
	gml::vec3 mCenter;
	float mRadius;
	float mRadiusSquare;	//caching
};

class Plane
{
public:
	Plane(const gml::vec3& position, const gml::vec3& normal);

	Plane& SetPosition(float x, float y, float z);

	Plane& SetPosition(const gml::vec3& position);

	Plane& SetNormal(const gml::vec3& normal);

	inline const gml::vec3& GetPosition() const { return mPosition; }

	inline const gml::vec3& GetNormal() const { return mNormal; }

private:
	gml::vec3 mPosition;
	gml::vec3 mNormal;
};

class Box
{
public:
	Box(const gml::vec3& center, const gml::vec3& extend);

	Box& SetCenter(float x, float y, float z);

	Box& SetCenter(const gml::vec3& center);

	Box& SetEntend(float w, float h, float d);

	Box& SetEntend(const gml::vec3& extend);

	Box& SetWidth(float width);

	Box& SetHeight(float height);

	Box& SetDepth(float depth);

	inline const gml::vec3& GetCenter() const { return mCenter; }

	inline const gml::vec3& GetExtend() const { return mExtend; }

	inline const gml::vec3& GetAxisX() const { return gml::vec3::right; }

	inline const gml::vec3& GetAxisY() const { return gml::vec3::up; }

	inline const gml::vec3& GetAxisZ() const { return gml::vec3::forward; }

	inline float GetWidth() const { return mExtend.x; }

	inline float GetHeight() const { return mExtend.y; }

	inline float GetDepth() const { return mExtend.z; }

private:
	gml::vec3 mCenter;
	gml::vec3 mExtend;
};

int IntersectPlaneWithRay(const gml::ray& ray, const gml::vec3& pV0, const gml::vec3& pNormal, bool dualFace, float& t0);
int IntersectTriangleWithRay(const gml::ray& ray, const gml::vec3& v0, const gml::vec3& v1, const gml::vec3& v2, float& t, float&u, float& v, gml::vec3& normal);
int Intersect(const gml::ray& ray, const Sphere& sphere, float& t0, float& t1);
int Intersect(const gml::ray& ray, const Plane& plane, float& t0);
int Intersect(const gml::ray& ray, const Box& box, float& t0, float& t1);
int Intersect(const gml::ray& ray, const gml::aabb& aabb);
