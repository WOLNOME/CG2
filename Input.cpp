#include "Input.h"
#include <cassert>
#include <wrl.h>
#include <windows.h>

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

using namespace Microsoft::WRL;

void Input::Initialize(HINSTANCE hInstance,HWND hwnd)
{
	HRESULT hr;

	//DirectInputの初期化
	IDirectInput8* directInput = nullptr;
	hr = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));

	//キーボードデバイスの生成
	IDirectInputDevice8* keyboard = nullptr;
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));
	//入力データ形式のセット
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));
	//排他制御レベルのセット
	hr = keyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	//ゲームパッドデバイスの生成
	IDirectInputDevice8* gamepad = nullptr;
	hr = directInput->CreateDevice(GUID_Joystick, &gamepad, NULL);
	assert(SUCCEEDED(hr));
	//入力データ形式のセット
	hr = gamepad->SetDataFormat(&c_dfDIJoystick);
	assert(SUCCEEDED(hr));
	// プロパティ設定(軸モードを絶対値モードとして設定)
	DIPROPDWORD diprop;
	ZeroMemory(&diprop, sizeof(diprop));
	diprop.diph.dwSize = sizeof(diprop);
	diprop.diph.dwHeaderSize = sizeof(diprop.diph);
	diprop.diph.dwHow = DIPH_DEVICE;
	diprop.diph.dwObj = 0;
	diprop.dwData = DIPROPAXISMODE_ABS;
	assert(SUCCEEDED(gamepad->SetProperty(DIPROP_AXISMODE, &diprop.diph)));
	// X軸の値の範囲設定
	DIPROPRANGE diprg;
	ZeroMemory(&diprg, sizeof(diprg));
	diprg.diph.dwSize = sizeof(diprg);
	diprg.diph.dwHeaderSize = sizeof(diprg.diph);
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.diph.dwObj = DIJOFS_X;
	diprg.lMin = -1000;
	diprg.lMax = 1000;
	assert(SUCCEEDED(gamepad->SetProperty(DIPROP_RANGE, &diprg.diph)));
	// Y軸の値の範囲設定
	diprg.diph.dwObj = DIJOFS_Y;
	assert(SUCCEEDED(gamepad->SetProperty(DIPROP_RANGE, &diprg.diph)));
	//協調モードの設定
	assert(SUCCEEDED(gamepad->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND)));

}

void Input::Update()
{

}
