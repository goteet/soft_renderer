#pragma once
#include <vector>
#include <iscene.h>
#include "geometry.h"
#include <gmlaabb.h>
#include <gmlcolor.h>

class SceneNode
{
public:
	static SceneNode* CreateRoot(float width, float height, float depth);

	~SceneNode();

	bool Add(ISceneObject* obj);

	ISceneObject* IntersectWithRay(const gml::ray& ray, HitInfo& info, ISceneObject* exclude) const;

	void Release();

private:
	SceneNode(SceneNode* parent = nullptr, int level = 0);
	gml::aabb GenerateChildAABB(int index);

	gml::aabb mAABB;
	int mLevel;
	SceneNode* mParent = nullptr;
	SceneNode* mChildren[8];
	std::vector<ISceneObject*> mObjects;

};

class Scene: public IScene
{
public:
	Scene();

	~Scene();

	virtual void Update();

	virtual ISceneObject* IntersectWithRay(const gml::ray& ray, HitInfo& info, ISceneObject* exclude) const;

	virtual const Light* GetLightList() const;

	virtual int GetLightCount() const;

	virtual const gml::color3& GetAmbientColor() const;

private:
	SceneNode* mSceneRoot = nullptr;
	std::vector<Light> mLights;
	float mRandomSeed;

	gml::color3 mAmbientColor = gml::color3(0.1f, 0.125f, 0.125f);
};