#include "pch.h"
#include "../resource/resource.h"
#include <time.h>
#include "renderer.h"

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HBITMAP CreateDIB(HDC dc, int width, int height, VOID** buffer);
bool Initialize(HWND hWnd);
void Uninitialize(HWND hWnd);
void CenterWindow(HWND hWnd);
namespace
{
	//窗口大小
	const int SCREEN_WIDTH = 800;
	const int SCREEN_HEIGHT = 600;

	const int COLOR_BIT = 24;
	const int LINE_PITCH = (SCREEN_WIDTH * COLOR_BIT + 31) / 32 * 4;
}

struct BufferBMP
{
	HBITMAP	hCanvas = NULL;
	HDC		hDC = NULL;
	byte*	pBuffer = NULL;
	int		Width = 0;
	int		Height = 0;
	int		Pitch = 0;

	bool Create(HDC dc, int width, int height)
	{
		hCanvas = CreateDIB(dc, width, height, (VOID**)&pBuffer);
		if (!hCanvas)
		{
			return false;
		}

		//创建dc
		hDC = ::CreateCompatibleDC(dc);
		::SelectObject(hDC, hCanvas);

		Width = width;
		Height = height;
		Pitch = (width * COLOR_BIT + 31) / 32 * 4;
		return true;
	}

	void Release()
	{
		pBuffer = NULL;

		::DeleteDC(hDC);
		hDC = NULL;

		DeleteObject(hCanvas);
		hCanvas = NULL;
	}
};

namespace
{
	const int MAX_LOADSTRING = 100;
	const wchar_t* APP_NAME = L"chi";

	HINSTANCE hInst;
	HWND hWindow;
	HDC hWindowDC;

	BufferBMP bufferBMP;
	Renderer* renderer = nullptr;
}

void Present(HDC hDC, BufferBMP bmp)
{
	::BitBlt(hDC, 0, 0, bufferBMP.Width, bufferBMP.Height, bmp.hDC, 0, 0, SRCCOPY);
}

void RenderScene()
{
	renderer->Render();
	renderer->CopyBuffer(bufferBMP.pBuffer, bufferBMP.Width, bufferBMP.Height, bufferBMP.Pitch);
	Present(hWindowDC, bufferBMP);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ wchar_t* lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow) ||
		!Initialize(hWindow))
	{
		return FALSE;
	}

	MSG msg;
	bool running = true;
	while (running)
	{
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				running = false;
				break;
			}
		}

		RenderScene();
	}

	Uninitialize(hWindow);
	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_CHI);
	wcex.lpszClassName = APP_NAME;
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	hWindow = CreateWindow(APP_NAME, APP_NAME, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWindow)
	{
		return FALSE;
	}

	ShowWindow(hWindow, nCmdShow);
	UpdateWindow(hWindow);
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool Initialize(HWND hWnd)
{
	srand(static_cast<unsigned>(time(0)));
	CenterWindow(hWnd);
	hWindowDC = ::GetDC(hWindow);

	//创建bmp容器
	if (!bufferBMP.Create(hWindowDC, SCREEN_WIDTH, SCREEN_HEIGHT))
	{
		return false;
	}

	renderer = new Renderer(SCREEN_WIDTH, SCREEN_HEIGHT);

	return true;
}

void Uninitialize(HWND hWnd)
{
	if (renderer != nullptr)
	{
		delete renderer;
		renderer = nullptr;
	}

	::ReleaseDC(hWindow, hWindowDC);
	bufferBMP.Release();
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
	if (halfWidth > centerX)halfWidth = centerX;
	if (halfHeight > centerY)halfHeight = centerY;

	::SetWindowPos(hWnd, NULL,
		centerX - halfWidth,
		centerY - halfHeight,
		SCREEN_WIDTH + 2 * borderWidth,
		SCREEN_HEIGHT + 2 * borderHeight, SWP_SHOWWINDOW);
}

HBITMAP CreateDIB(HDC dc, int width, int height, VOID** buffer)
{
	if (width <= 0 || height <= 0)
	{
		return NULL;
	}

	BITMAPINFO bmpInfo;
	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biWidth = width;
	bmpInfo.bmiHeader.biHeight = -height;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = COLOR_BIT;
	bmpInfo.bmiHeader.biCompression = BI_RGB;

	HBITMAP bmp = ::CreateDIBSection(
		dc, &bmpInfo,
		DIB_RGB_COLORS,
		buffer,
		NULL, 0);

	return bmp;
}