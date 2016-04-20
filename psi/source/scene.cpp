#include "pch.h"
#include <isceneobject.h>
#include "scene.h"


IScene* IScene::Create()
{
	return new Scene();
}

IScene::~IScene()
{

}

void IScene::Release()
{
	delete this;
}

SceneNode* SceneNode::CreateRoot(float width, float height, float depth)
{
	SceneNode* node = new SceneNode();
	node->mAABB.expand(gml::vec3(-width, -height, -depth));
	node->mAABB.expand(gml::vec3(+width, +height, +depth));
	return node;
}
SceneNode::SceneNode(SceneNode* parent, int level) : mParent(parent), mLevel(level)
{
	for (int i = 0; i < 8; i++)
	{
		mChildren[i] = nullptr;
	}
}

SceneNode::~SceneNode()
{
	for (int i = 0; i < 8; i++)
	{
		if (mChildren[i] != nullptr)
		{
			mChildren[i]->Release();
		}
	}

	for (int i = 0, length = mObjects.size(); i < length; ++i)
	{
		mObjects[i]->Release();
	}

}

void SceneNode::Release()
{
	delete this;
}

bool SceneNode::Add(ISceneObject* obj)
{
	const gml::aabb& aabb = obj->GetAABB();
	if (!mAABB.is_contain(aabb))
	{
		if (mParent == nullptr)
		{
			mObjects.push_back(obj);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (mLevel < 3)
		{
			for (int i = 0; i < 8; i++)
			{
				if (mChildren[i] == nullptr)
				{
					gml::aabb aabb = GenerateChildAABB(i);
					if (aabb.is_contain(obj->GetAABB()))
					{
						mChildren[i] = new SceneNode(this, mLevel + 1);
						mChildren[i]->mAABB = aabb;
					}
				}

				if (mChildren[i] != nullptr && mChildren[i]->Add(obj))
				{
					return true;
				}
			}
		}

		mObjects.push_back(obj);
		return true;
	}
}

gml::aabb SceneNode::GenerateChildAABB(int index)
{
	gml::aabb aabb;
	aabb.expand(mAABB.center());
	gml::vec3 v;
	switch (index)
	{
	case 0:v.set(mAABB.min_bound().x, mAABB.min_bound().y, mAABB.min_bound().z); break;
	case 1:v.set(mAABB.max_bound().x, mAABB.min_bound().y, mAABB.min_bound().z); break;
	case 2:v.set(mAABB.min_bound().x, mAABB.max_bound().y, mAABB.min_bound().z); break;
	case 3:v.set(mAABB.max_bound().x, mAABB.max_bound().y, mAABB.min_bound().z); break;
	case 4:v.set(mAABB.min_bound().x, mAABB.min_bound().y, mAABB.max_bound().z); break;
	case 5:v.set(mAABB.max_bound().x, mAABB.min_bound().y, mAABB.max_bound().z); break;
	case 6:v.set(mAABB.min_bound().x, mAABB.max_bound().y, mAABB.max_bound().z); break;
	case 7:v.set(mAABB.max_bound().x, mAABB.max_bound().y, mAABB.max_bound().z); break;
	}
	aabb.expand(v);
	return aabb;
}

ISceneObject* SceneNode::IntersectWithRay(const gml::ray& ray, HitInfo& info, ISceneObject* exclude) const
{
	if (mParent == nullptr || Intersect(ray, mAABB) > 0)
	{
		ISceneObject* hitObject = nullptr;
		for (int i = 0, length = mObjects.size(); i < length; ++i)
		{
			ISceneObject* object = mObjects[i];
			if (object == exclude)
				continue;

			if (object->IntersectWithRay(ray, info.t, info))
			{
				hitObject = object;
			}
		}

		ISceneObject* testHitObject = nullptr;
		for (int c = 0; c < 8; c++)
		{
			if (mChildren[c] != nullptr)
			{
				testHitObject = mChildren[c]->IntersectWithRay(ray, info, exclude);
				if (testHitObject != nullptr)
				{
					hitObject = testHitObject;
				}
			}
		}

		return hitObject;
	}
	return nullptr;
}


Scene::Scene()
{
	mSceneRoot = SceneNode::CreateRoot(200, 200, 200);

	if (1)		//sphere
	{
		const int LINE_COUNT = 2;
		const int OBJ_COUNT = LINE_COUNT * LINE_COUNT;
		const float SIZE = 3.0f;
		const float INTERVAL = SIZE * 2 + 1.0f;

		float offset = -(LINE_COUNT - 1) * 0.5f * INTERVAL;
		for (int i = 0; i < LINE_COUNT; ++i)
		{
			for (int j = 0; j < LINE_COUNT; ++j)
			{
				{
					ISceneObject* sphere = ISceneObject::CreateSphere(gml::vec3(offset + i * INTERVAL, offset + j * INTERVAL, (j % 2) * -10.0f - 60.0f + offset), SIZE);
					sphere->GetMaterial()->IsReflective = i % 2 == 0;
					sphere->GetMaterial()->IsTransparent = j % 2 == 0;
					mSceneRoot->Add(sphere);
				}
			}
		}
	}

	if (1)		//box
	{
		ISceneObject* box = ISceneObject::CreateBox(gml::vec3(-30, -10, -90), gml::vec3(4, 2, 7));
		mSceneRoot->Add(box);
	}

	if (1)		//pyramid
	{
		ISceneObject* pyramid = ISceneObject::CreatePyramid(gml::vec3(20, 5, -65), 4);
		mSceneRoot->Add(pyramid);
	}

	if (0)		//model
	{
		ISceneObject* model = ISceneObject::CreateModel(gml::vec3(-5, -15, -90), 2.5f);
		mSceneRoot->Add(model);
	}

	if (1)		//wall
	{
		ISceneObject* wall;

		wall = ISceneObject::CreatePlane(gml::vec3(0, 0, -100), gml::vec3(0, 0, 1));
		mSceneRoot->Add(wall);

		wall = ISceneObject::CreatePlane(gml::vec3(-40, 0, 0), gml::vec3(1, 0, 0));
		mSceneRoot->Add(wall);

		wall = ISceneObject::CreatePlane(gml::vec3(40, 0, 0), gml::vec3(-1, 0, 0));
		mSceneRoot->Add(wall);

		wall = ISceneObject::CreatePlane(gml::vec3(0, -15, 0), gml::vec3(0, 1, 0));
		mSceneRoot->Add(wall);
	}

	//light
	mLights.resize(2);

	mLights[0].Color.set(0.5f, 0.2f, 1.0f);
	mLights[0].Intensity = 0.35f;
	mLights[1].Color.set(1.0f, 0.6f, 0.6f);
	mLights[1].Position.set(0, 50, -60);
	mLights[1].Intensity = 0.75f;

	mRandomSeed = 0.5f;
}

Scene::~Scene()
{
	mSceneRoot->Release();
}


ISceneObject* Scene::IntersectWithRay(const gml::ray&ray, HitInfo& info, ISceneObject* exclude) const
{
	info.t = FLT_MAX;
	return mSceneRoot->IntersectWithRay(ray, info, exclude);
}

void Scene::Update()
{
	const float pi2 = 3.141592653f * 2.0f;
	const float R = 35.0f;

	mRandomSeed += 0.005f;
	if (mRandomSeed > 1.0f)
	{
		mRandomSeed -= 1.0f;
	}

	float radius = pi2 * mRandomSeed;

	float coss = R * cos(radius);
	float sins = R * sin(radius);


	mLights[0].Position.set(coss, 0, -50 + sins);
}

const Light* Scene::GetLightList() const
{
	return &(mLights[0]);
}

int Scene::GetLightCount() const
{
	return mLights.size();
}

const gml::color3& Scene::GetAmbientColor() const
{
	return mAmbientColor;
}