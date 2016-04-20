#include "pch.h"
#include <thread>
#include <iscene.h>
#include "renderer.h"
#include <gml/include/math_util.h>
#include <gml/include/ray.h>
#include <gml/include/color.h>


IRenderer* IRenderer::Create()
{
	return new Renderer();
}

IRenderer::~IRenderer()
{

}

void IRenderer::Release()
{
	delete this;
}

struct PresentStuff
{
	std::thread thread;

	int xStart;
	int xEnd;
	int yStart;
	int yEnd;

	int pitch;
	int width;
	int height;

	unsigned char* canvas;

	void Join()
	{
		thread.join();
	}
};

namespace
{
	const int RECCURSIVE_DEPTH = 4;

	const int THREAD_ROW = 2;
	const int THREAD_COL = 4;
	const int THREAD_COUNT = THREAD_ROW * THREAD_COL;

	PresentStuff PresentThreads[THREAD_COUNT];
}

void Renderer::Present(const IScene* scene, unsigned char* canvas, int width, int height, int pitch)
{
	int xSeg = width / THREAD_COL;
	int ySeg = height / THREAD_ROW;

	for (int i = 0; i < THREAD_ROW; i++)
	{
		for (int j = 0; j < THREAD_COL; j++)
		{
			int index = j + i * THREAD_COL;
			PresentThreads[index].xStart = j * xSeg;
			PresentThreads[index].yStart = i * ySeg;
			PresentThreads[index].xEnd = (j + 1) * xSeg;
			PresentThreads[index].yEnd = (i + 1) * ySeg;

			PresentThreads[index].pitch = pitch;
			PresentThreads[index].width = width;
			PresentThreads[index].height = height;
			PresentThreads[index].canvas = canvas;

			if (PresentThreads[index].xEnd > width)
				PresentThreads[index].xEnd = width;
			if (PresentThreads[index].yEnd > height)
				PresentThreads[index].yEnd = height;

			PresentThreads[index].thread = std::thread(&Renderer::InternalPresent, this, &(PresentThreads[index]), scene);
		}
	}
	for (auto& t : PresentThreads)
	{
		t.Join();
	}
}

gml::color3 Renderer::Trace(const IScene* scene, const gml::ray& ray, int reccursiveDepth)
{
	const float BIAS = 1e-3f;
	if (reccursiveDepth > RECCURSIVE_DEPTH)
	{
		return mClearColor;
	}

	HitInfo t;
	ISceneObject* hitObject = scene->IntersectWithRay(ray, t);
	if (hitObject == nullptr)
	{
		return mClearColor;
	}

	gml::vec3 intersectPosition = ray.get_pos_by_len(t.t);
	gml::color3 color;
	const Material* material = hitObject->GetMaterial();
	if (material->IsReflective && reccursiveDepth < RECCURSIVE_DEPTH)	//·´Éä
	{
		bool isInside = false;
		if (dot(t.normal, ray.direction()) > 0)
		{
			isInside = true;
			t.normal = -t.normal;
		}

		float facingRatio = dot(t.normal, -ray.direction());
		float fresnel = gml::lerp(pow(1.0f - facingRatio, 2.5f), 1.0f, 0.05f);

		float ior = 1.1f;
		float eta = isInside ? ior : 1.0f / ior;
		float cosi = dot(-t.normal, ray.direction());
		float cosr = 1.0f - eta * eta * (1.0f - cosi * cosi);

		gml::vec3 biasNormal = t.normal * BIAS;
		gml::ray reflectRay;
		reflectRay.set_origin(intersectPosition + biasNormal);
		reflectRay.set_dir(ray.direction() - 2 * t.normal * dot(t.normal, ray.direction()));
		gml::color3 reflectColor = Trace(scene, reflectRay, reccursiveDepth + 1);

		if (material->IsTransparent)
		{
			gml::ray refractRay;
			refractRay.set_origin(intersectPosition - biasNormal);
			refractRay.set_dir(ray.direction() * eta + t.normal * (eta * cosi - sqrt(cosr)));
			gml::color3 refractColor = Trace(scene, refractRay, reccursiveDepth + 1);

			color = lerp(refractColor, reflectColor, fresnel);
		}
		else
		{
			gml::color3 surfaceColor = scene->GetAmbientColor();

			gml::ray shadowRay;
			shadowRay.set_origin(intersectPosition + t.normal * BIAS);

			for (int l = 0, lightCount = scene->GetLightCount(); l < lightCount; ++l)
			{
				const Light& light = scene->GetLightList()[l];
				gml::vec3 Point2Light = light.Position - intersectPosition;
				float distance2 = Point2Light.length_sqr();
				shadowRay.set_dir(Point2Light);

				HitInfo st;
				bool shadow = false;
				ISceneObject* shadowHitObject = scene->IntersectWithRay(shadowRay, st);
				if (shadowHitObject != nullptr && (st.t * st.t) < distance2)
				{
					shadow = true;
				}
				if (!shadow)
				{
					float cosS = dot(t.normal, shadowRay.direction());
					if (cosS < 0)
						cosS = 0.0f;

					surfaceColor += light.Color * cosS * light.Intensity;
				}
			}
			color = lerp(surfaceColor, reflectColor, 0.02f);
		}

	}
	else
	{
		color = scene->GetAmbientColor();
		gml::ray shadowRay;
		shadowRay.set_origin(intersectPosition + t.normal * BIAS);

		for (int l = 0, lightCount = scene->GetLightCount(); l < lightCount; ++l)
		{
			const Light& light = scene->GetLightList()[l];
			gml::vec3 Point2Light = light.Position - intersectPosition;
			float distance2 = Point2Light.length_sqr();
			shadowRay.set_dir(Point2Light);

			HitInfo st;
			bool shadow = false;
			ISceneObject* shadowHitObject = scene->IntersectWithRay(shadowRay, st);
			if (shadowHitObject != nullptr && (st.t * st.t) < distance2)
			{
				shadow = true;
			}
			if (!shadow)
			{
				float cosS = dot(t.normal, shadowRay.direction());
				if (cosS < 0)
					cosS = 0.0f;

				color += light.Color * cosS * light.Intensity;
			}
		}
	}

	color.clamp();
	return color;
}


void Renderer::InternalPresent(PresentStuff* seg, const IScene* scene)
{
	int index;
	gml::color3 color;

	int indexOffset = seg->height - 1;
	for (int y = seg->yStart; y < seg->yEnd; y++)
	{
		for (int x = seg->xStart; x < seg->xEnd; x++)
		{
			gml::ray ray = mCamera.GenerateRay(seg->width, seg->height, x, y);
			color = Trace(scene, ray, 0);

			index = x * 3 + (indexOffset - y) * seg->pitch;
			unsigned int color_rgb = color.to_rgb();
			seg->canvas[index + 0] = color_rgb & 0xFF;  //r
			seg->canvas[index + 1] = (color_rgb >> 8) & 0xFF; //g
			seg->canvas[index + 2] = (color_rgb >> 16) & 0xFF; //b
		}
	}
}