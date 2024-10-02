#pragma once
#include <windows.h>
#include <wrl.h>

class WinApp
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
public:
	void Initialize();
	void Update();


};

