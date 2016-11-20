#pragma once
#include <material.h>
#include <gmlaabb.h>
#include <gmlray.h>

class HitInfo
{
public:
	float t;
	gml::vec3 normal;
};

class ISceneObject
{
public:
	static ISceneObject* CreateSphere(const gml::vec3& position, float radius);
	static ISceneObject* CreatePlane(const gml::vec3& position, const gml::vec3& normal);
	static ISceneObject* CreateBox(const gml::vec3& position, const gml::vec3& extends);
	static ISceneObject* CreatePyramid(const gml::vec3& position, float extends);
	static ISceneObject* CreateModel(const gml::vec3& position, float size);

	virtual ~ISceneObject();

	virtual void Release();

	virtual bool IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const = 0;

	virtual const gml::aabb& GetAABB() const = 0;

	virtual void SetPosition(float x, float y, float z) = 0;

	virtual void SetPosition(const gml::vec3& center) = 0;

	virtual const gml::vec3& GetPosition() const = 0;

	virtual Material* GetMaterial() = 0;

	virtual const Material* GetMaterial() const = 0;
};