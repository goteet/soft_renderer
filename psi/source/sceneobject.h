#pragma once
#include <isceneobject.h>
#include <material.h>
#include "geometry.h"
#include <gml/include/aabb.h>

class SceneObject : public ISceneObject
{
public:
	SceneObject(const SceneObject&) = delete;
	SceneObject& operator = (const SceneObject&) = delete;
	
	virtual Material* GetMaterial();

	virtual const Material* GetMaterial() const;

	virtual const gml::aabb& GetAABB() const { return mAABB; }

protected:
	SceneObject();

	Material mMaterial;
	gml::aabb mAABB;

	
};

class SphereSceneObject : public SceneObject
{
public:
	SphereSceneObject(const gml::vec3& position, float radius);
	
	virtual bool IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const;

	virtual void SetPosition(float x, float y, float z);

	virtual void SetPosition(const gml::vec3& center);

	virtual const gml::vec3& GetPosition() const;

private:
	Sphere mSphere;
};

class PlaneSceneObject : public SceneObject
{
public:
	PlaneSceneObject(const gml::vec3& position, const gml::vec3& normal);

	virtual bool IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const;

	virtual void SetPosition(float x, float y, float z);

	virtual void SetPosition(const gml::vec3& center);

	virtual const gml::vec3& GetPosition() const;

private:
	Plane mPlane;
};

class BoxSceneObject : public SceneObject
{
public:
	BoxSceneObject(const gml::vec3& position, const gml::vec3& extend);

	virtual bool IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const;

	virtual void SetPosition(float x, float y, float z);

	virtual void SetPosition(const gml::vec3& center);

	virtual const gml::vec3& GetPosition() const;

private:
	Box mBox;
};

class PyramidSceneObject : public SceneObject
{
public:
	PyramidSceneObject(const gml::vec3& position, float extend);

	virtual bool IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const;

	virtual void SetPosition(float x, float y, float z);

	virtual void SetPosition(const gml::vec3& center);

	virtual const gml::vec3& GetPosition() const;

private:
	gml::vec3 mCenter;
	gml::vec3 mVerts[4];

public:
	static const gml::vec3 VERTS[4];
	static const int FACE[12];
};

class ModelSceneObject : public SceneObject
{
public:
	ModelSceneObject(const gml::vec3& position, float size);

	virtual bool IntersectWithRay(const gml::ray& ray, float mint, HitInfo& info) const;

	virtual void SetPosition(float x, float y, float z);

	virtual void SetPosition(const gml::vec3& center);

	virtual const gml::vec3& GetPosition() const;

private:
	gml::vec3 mCenter;
	gml::vec3 mVerts[138];
};