#pragma once

class IScene;

class IRenderer
{
public:
	static IRenderer* Create();

	virtual ~IRenderer();

	virtual void Release(); 

	virtual void Present(const IScene* scene, unsigned char* buffer, int width, int height, int pitch) = 0;
};