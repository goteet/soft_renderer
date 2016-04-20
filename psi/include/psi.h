#pragma once

void Present(HDC windowDC);
bool Initialize(HWND hWnd);
void Uninitialize(HWND hWnd);
bool NeedUpdate();