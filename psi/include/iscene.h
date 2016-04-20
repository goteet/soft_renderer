#pragma once
namespace gml
{
	class color3;
	class vec3;
	class ray;
}

class HitInfo;
class ISceneObject;
class Light;

class IScene
{
public:
	static IScene* Create();

	virtual ~IScene();

	virtual void Release();

	virtual void Update() = 0;

	virtual ISceneObject* IntersectWithRay(const gml::ray& ray, HitInfo& info, ISceneObject* exclude = nullptr) const = 0;

	virtual const Light* GetLightList() const = 0;

	virtual int GetLightCount() const = 0;

	virtual const gml::color3& GetAmbientColor() const = 0;
};