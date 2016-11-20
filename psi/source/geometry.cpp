#include "pch.h"
#include <math.h>
#include <gmlray.h>
#include <gmlaabb.h>
#include "geometry.h"

//////////////////////////////////////////////////////////
//
Sphere::Sphere(const gml::vec3& center, float r) :mCenter(center)
{
	SetRadius(r);
}

Sphere& Sphere::SetCenter(float x, float y, float z)
{
	mCenter.set(x, y, z);
	return *this;
}

Sphere& Sphere::SetCenter(const gml::vec3& center)
{
	mCenter = center;
	return *this;
}

Sphere& Sphere::SetRadius(float r)
{
	if (r < 0)
		r = 0;
	mRadius = r;
	mRadiusSquare = mRadius * mRadius;
	return *this;
}

//////////////////////////////////////////////////////////
//
Plane::Plane(const gml::vec3& position, const gml::vec3& normal) :mPosition(position)
{
	SetNormal(normal);
}

Plane& Plane::SetPosition(float x, float y, float z)
{
	mPosition.set(x, y, z);
	return *this;
}

Plane& Plane::SetPosition(const gml::vec3& position)
{
	mPosition = position;
	return *this;
}

Plane& Plane::SetNormal(const gml::vec3& normal)
{
	mNormal = normal.normalized();
	return *this;
}
//////////////////////////////////////////////
//
Box::Box(const gml::vec3& center, const gml::vec3& extend) :mCenter(center), mExtend(extend)
{

}

Box& Box::SetCenter(float x, float y, float z)
{
	mCenter.set(x, y, z);
	return *this;
}

Box& Box::SetCenter(const gml::vec3& center)
{
	mCenter = center;
	return *this;
}

Box& Box::SetEntend(float w, float h, float d)
{
	mExtend.set(w, h, d);
	return *this;
}

Box& Box::SetEntend(const gml::vec3& extend)
{
	mExtend = extend;
	return *this;
}

Box& Box::SetWidth(float width)
{
	mExtend.x = width;
	return *this;
}

Box& Box::SetHeight(float height)
{
	mExtend.y = height;
	return *this;
}

Box& Box::SetDepth(float depth)
{
	mExtend.z = depth;
	return *this;
}

//////////////////////////////////////////////////////////
//
int IntersectPlaneWithRay(const gml::ray& ray, const gml::vec3& P0, const gml::vec3& pNormal, bool dualFace, float& t0)
{
	gml::vec3 p0o = P0 - ray.origin();
	float dotDN = dot(ray.direction(), pNormal);

	if (dotDN == 0.0f)//平行
	{
		return 0;
	}
	else if (!dualFace && dotDN > 0.0f) //射线平面同向，将会从平面背面相交
	{
		return 0;
	}

	t0 = dot(p0o, pNormal) / dotDN;
	return (t0 >= 0.0f) ? 1 : 0;
}

int IntersectTriangleWithRay(const gml::ray& ray, const gml::vec3& v0, const gml::vec3& v1, const gml::vec3& v2, float& t, float&u, float& v, gml::vec3& normal)
{
	//moller-trumbore

	gml::vec3 v1v0 = v1 - v0;
	gml::vec3 v2v0 = v2 - v0;

	gml::vec3 pv = cross(ray.direction(), v2v0);
	float det = dot(v1v0, pv);
	if (det == 0.0f)// 平行
	{
		return 0;
	}

	float invDet = 1.0f / det;

	gml::vec3 tv = ray.origin() - v0;
	u = dot(tv, pv) * invDet;
	if (u < 0.0f || u > 1.0f)//三角形外
	{
		return 0;
	}

	gml::vec3 qv = cross(tv, v1v0);
	v = dot(ray.direction(), qv) * invDet;
	if (v < 0.0f || u + v > 1.0f)		//三角形外
	{
		return 0;
	}

	t = dot(v2v0, qv) * invDet;
	if (t < 0.0f)
	{
		return 0;
	}

	normal = cross(v2v0, v1v0).normalized();
	return 1;

}

int Intersect(const gml::ray& ray, const Sphere& sphere, float& t0, float& t1)
{
	gml::vec3 CO = sphere.GetCenter() - ray.origin();
	float dotRCO = dot(ray.direction(), CO);

	if (dotRCO < 0.0f)
	{
		return 0;
	}

	float co2 = dot(CO, CO);
	float distance2 = co2 - dotRCO * dotRCO;

	if (distance2 > sphere.GetRadiusSquare())
	{
		return 0;
	}
	else if (distance2 == sphere.GetRadiusSquare())
	{
		t0 = t1 = dotRCO;
		return 1;
	}
	else
	{
		float discriminent = static_cast<float>(sqrt(sphere.GetRadiusSquare() - distance2));
		t0 = dotRCO - discriminent;
		t1 = dotRCO + discriminent;
		if (t0 < 0)
		{
			t0 = t1;
			return 1;
		}
		else
		{
			return 2;
		}
	}
}

int Intersect(const gml::ray& ray, const Plane& plane, float& t0)
{
	return IntersectPlaneWithRay(ray, plane.GetPosition(), plane.GetNormal(), true, t0);
}

int Intersect(const gml::ray& ray, const Box& box, float& t0, float& t1)
{
	int parallelMask = 0;
	bool found = false;

	gml::vec3 dirDotAxis;
	gml::vec3 ocDotAxis;

	gml::vec3 oc = box.GetCenter() - ray.origin();
	gml::vec3 axis[3] = { box.GetAxisX(), box.GetAxisY(), box.GetAxisZ() };
	float extend[3] = { box.GetExtend().x, box.GetExtend().y, box.GetExtend().z };

	for (int i = 0; i < 3; ++i)
	{
		dirDotAxis[i] = dot(ray.direction(), axis[i]);
		ocDotAxis[i] = dot(oc, axis[i]);

		if (dirDotAxis[i] == 0.0f)
		{
			//垂直一个方向，说明与这个方向为法线的平面平行。
			//先不处理，最后会判断是否在两个平面的区间内
			parallelMask |= 1 << i;
		}
		else
		{
			float es = (dirDotAxis[i] > 0.0f) ? extend[i] : -extend[i];
			float invDA = 1.0f / dirDotAxis[i];	//这个作为cos来使用，为了底下反算某轴向方向到 中心连线方向的长度

			if (!found)
			{
				t0 = (ocDotAxis[i] - es) * invDA;
				t1 = (ocDotAxis[i] + es) * invDA;
				found = true;
			}
			else
			{
				float s = (ocDotAxis[i] - es) * invDA;
				if (s > t0)
				{
					t0 = s;
				}

				s = (ocDotAxis[i] + es) * invDA;
				if (s < t1)
				{
					t1 = s;
				}

				if (t0 > t1)
				{
					//这里 intersect0代表就近点, intersect1代表远点。
					//t0 > t1，亦近点比远点大
					//表明了 两个t 都是负数。
					//说明了obb是在射线origin的反方向上。
					//或者是在偏移到外部擦身而过了
					return 0;
				}
			}
		}
	}

	if (parallelMask)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (parallelMask & (1 << i))
			{
				if (fabs(ocDotAxis[i] - t0 * dirDotAxis[i]) > extend[i] ||
					fabs(ocDotAxis[i] - t1 * dirDotAxis[i]) > extend[i])
				{
					return 0;
				}
			}
		}
	}

	if (t1 < 0)
	{
		return 0;
	}
	else if (t0 < 0)
	{
		t0 = t1;
		return 1;
	}
	else
	{
		return 2;
	}
}


int Intersect(const gml::ray& ray, const gml::aabb& aabb)
{
	gml::vec3 invDir = ray.direction().inversed();
	float tMin, tMax, tMinY, tMaxY, tMinZ, tMaxZ;

	if (invDir.x >= 0.0f)
	{
		tMin = (aabb.min_bound().x - ray.origin().x) * invDir.x;
		tMax = (aabb.max_bound().x - ray.origin().x) * invDir.x;
	}
	else
	{
		tMax = (aabb.min_bound().x - ray.origin().x) * invDir.x;
		tMin = (aabb.max_bound().x - ray.origin().x) * invDir.x;
	}

	if (invDir.y >= 0.0f)
	{
		tMinY = (aabb.min_bound().y - ray.origin().y) * invDir.y;
		tMaxY = (aabb.max_bound().y - ray.origin().y) * invDir.y;
	}
	else
	{
		tMaxY = (aabb.min_bound().y - ray.origin().y) * invDir.y;
		tMinY = (aabb.max_bound().y - ray.origin().y) * invDir.y;
	}


	if ((tMin > tMaxY) || (tMinY > tMax))
	{
		return false;
	}

	if (tMinY > tMin)	tMin = tMinY;
	if (tMaxY < tMax)	tMax = tMaxY;

	if (invDir.z >= 0.0f)
	{
		tMinZ = (aabb.min_bound().z - ray.origin().z) * invDir.z;
		tMaxZ = (aabb.max_bound().z - ray.origin().z) * invDir.z;
	}
	else
	{
		tMaxZ = (aabb.min_bound().z - ray.origin().z) * invDir.z;
		tMinZ = (aabb.max_bound().z - ray.origin().z) * invDir.z;
	}

	if ((tMin > tMaxZ) || (tMinZ > tMax))
	{
		return false;
	}

	if (tMinZ > tMin)		tMin = tMinZ;
	if (tMaxZ < tMax)		tMax = tMaxZ;

	return true;
}