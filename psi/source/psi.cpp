#pragma once
#include "pch.h"
#include <time.h>
#include <thread>
#include "psi.h"
#include "irenderer.h"
#include "iscene.h"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const int RGB_BIT = 24;
const int PITCH = (SCREEN_WIDTH * RGB_BIT + 31) / 32 * 4;

void CenterWindow(HWND hWnd);
HBITMAP CreateDIB(HDC dc, int width, int height, VOID** buffer);
HDC	bmpDC;
HBITMAP canvas = NULL;

unsigned char* colorBuffer = nullptr;
unsigned char* canvasPtr = nullptr;
IRenderer* renderer = nullptr;
IScene* scene = nullptr;


std::thread renderThread;
volatile bool needRenderScene = true;
volatile bool needUpdateCanvas = false;


void RenderScene()
{
	while (needRenderScene)
	{
		scene->Update();

		//自旋等待
		while (needUpdateCanvas);

		renderer->Present(scene, colorBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH * 3);
		needUpdateCanvas = true;
	}

}


bool Initialize(HWND hWnd)
{
	srand(static_cast<unsigned>(time(0)));
	CenterWindow(hWnd);

	//创建bmp容器
	HDC dc = ::GetDC(hWnd);
	canvas = CreateDIB(dc, SCREEN_WIDTH, SCREEN_HEIGHT, (VOID**)&canvasPtr);
	if (!canvas)
	{
		return false;
	}

	//创建dc
	bmpDC = CreateCompatibleDC(dc);
	SelectObject(bmpDC, canvas);
	::ReleaseDC(hWnd, dc);

	colorBuffer = new unsigned char[SCREEN_WIDTH * SCREEN_HEIGHT * 3];

	renderer = IRenderer::Create();
	scene = IScene::Create();

	needRenderScene = true;
	renderThread = std::thread(RenderScene);
	return true;
}


bool NeedUpdate()
{
	if (needUpdateCanvas)
	{
		//拷贝
		int canvasIndex;
		int bufferIndex;
		for (int y = 0; y < SCREEN_HEIGHT; ++y)
		{
			for (int x = 0; x < SCREEN_WIDTH; ++x)
			{
				canvasIndex = x * 3 + y * PITCH;
				bufferIndex = 3 * (x + y * SCREEN_WIDTH);
				for (int i = 0; i < 3; i++)
				{
					canvasPtr[canvasIndex++] = colorBuffer[bufferIndex++];
				}
			}
		}
		needUpdateCanvas = false;
		return true;
	}

	return false;
}

void Present(HDC windowDC)
{
	::BitBlt(windowDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bmpDC, 0, 0, SRCCOPY);
}

void Uninitialize(HWND hWnd)
{
	needRenderScene = false;
	renderThread.join();

	::DeleteDC(bmpDC);
	DeleteObject(canvas);

	if (renderer != nullptr)
	{
		renderer->Release();
		renderer = nullptr;
	}

	if (scene != nullptr)
	{
		scene->Release();
		scene = nullptr;
	}

	if (colorBuffer != nullptr)
	{
		delete[] colorBuffer;
		colorBuffer = nullptr;
	}
}



void CenterWindow(HWND hWnd)
{
	RECT clientRect;
	RECT windowRect;
	::GetClientRect(hWnd, &clientRect);
	::GetWindowRect(hWnd, &windowRect);


	int borderWidth = (windowRect.right - windowRect.left - clientRect.right) / 2;
	int borderHeight = (windowRect.bottom - windowRect.top - clientRect.bottom) / 2;

	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY), &mi);
	int centerX = (mi.rcWork.right + mi.rcWork.left) / 2;
	int centerY = (mi.rcWork.top + mi.rcWork.bottom) / 2;

	int halfWidth = SCREEN_WIDTH / 2 + borderWidth;
	int halfHeight = SCREEN_HEIGHT / 2 + borderHeight;
	::SetWindowPos(hWnd, NULL,
		centerX - halfWidth,
		centerY - halfHeight,
		SCREEN_WIDTH + 2 * borderWidth,
		SCREEN_HEIGHT + 2 * borderHeight, SWP_SHOWWINDOW);
}

HBITMAP CreateDIB(HDC dc, int width, int height, VOID** buffer)
{
	if (width == 0)
	{
		width = SCREEN_WIDTH;
	}

	if (height == 0)
	{
		height = SCREEN_HEIGHT;
	}

	BITMAPINFO bmpInfo;
	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biHeight = -height;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = RGB_BIT;
	bmpInfo.bmiHeader.biCompression = BI_RGB;


	HBITMAP bmp = ::CreateDIBSection(
		dc, &bmpInfo,
		DIB_RGB_COLORS,
		buffer,
		NULL, 0);

	return bmp;
}