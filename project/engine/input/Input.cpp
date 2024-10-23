#include "Input.h"
#include <cassert>
#include <Windows.h>
#include "WinApp.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")


void Input::Initialize(WinApp* winApp)
{
	//借りてきたWinAppのインスタンスを記憶
	winApp_ = winApp;

	HRESULT hr;
	//DirectInputの初期化
	IDirectInput8* directInput = nullptr;
	hr = DirectInput8Create(
		winApp_->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));

	//キーボードデバイスの生成
	keyboard = nullptr;
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));
	//入力データ形式のセット
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));
	//排他制御レベルのセット
	hr = keyboard->SetCooperativeLevel(
		winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	//ゲームパッドデバイスの生成
	//gamepad = nullptr;
	//hr = directInput->CreateDevice(GUID_Joystick, &gamepad, NULL);
	//assert(SUCCEEDED(hr));
	////入力データ形式のセット
	//hr = gamepad->SetDataFormat(&c_dfDIJoystick);
	//assert(SUCCEEDED(hr));
	//// プロパティ設定(軸モードを絶対値モードとして設定)
	//DIPROPDWORD diprop;
	//ZeroMemory(&diprop, sizeof(diprop));
	//diprop.diph.dwSize = sizeof(diprop);
	//diprop.diph.dwHeaderSize = sizeof(diprop.diph);
	//diprop.diph.dwHow = DIPH_DEVICE;
	//diprop.diph.dwObj = 0;
	//diprop.dwData = DIPROPAXISMODE_ABS;
	//assert(SUCCEEDED(gamepad->SetProperty(DIPROP_AXISMODE, &diprop.diph)));
	//// X軸の値の範囲設定
	//DIPROPRANGE diprg;
	//ZeroMemory(&diprg, sizeof(diprg));
	//diprg.diph.dwSize = sizeof(diprg);
	//diprg.diph.dwHeaderSize = sizeof(diprg.diph);
	//diprg.diph.dwHow = DIPH_BYOFFSET;
	//diprg.diph.dwObj = DIJOFS_X;
	//diprg.lMin = -1000;
	//diprg.lMax = 1000;
	//assert(SUCCEEDED(gamepad->SetProperty(DIPROP_RANGE, &diprg.diph)));
	//// Y軸の値の範囲設定
	//diprg.diph.dwObj = DIJOFS_Y;
	//assert(SUCCEEDED(gamepad->SetProperty(DIPROP_RANGE, &diprg.diph)));
	////協調モードの設定
	//assert(SUCCEEDED(gamepad->SetCooperativeLevel(winApp_->GetHwnd(), DISCL_EXCLUSIVE | DISCL_FOREGROUND)));

}

void Input::Update()
{
	HRESULT hr;

	//前回のキー入力を保存
	memcpy(preKey, key, sizeof(key));

	//グラフィックスコマンド
	keyboard->Acquire();
	//全キーの入力状態を取得する
	keyboard->GetDeviceState(sizeof(key), key);

	//ゲームパッドのグラフィックスコマンド
	//gamepad->Acquire();
	//gamepad->Poll();
	//DIJOYSTATE padData;
	////デバイス取得
	//hr = gamepad->GetDeviceState(sizeof(DIJOYSTATE), &padData);
	//assert(SUCCEEDED(hr));
	////スティック判定
	//bool isPush[ButtonKind::ButtonKindMax];
	//for (int i = 0; i < ButtonKind::ButtonKindMax; i++) {
	//	isPush[i] = false;
	//}
	//int unresponsive_range = 200;
	//if (padData.lX < -unresponsive_range)
	//{
	//	isPush[ButtonKind::LeftButton] = true;
	//}
	//else if (padData.lX > unresponsive_range)
	//{
	//	isPush[ButtonKind::RightButton] = true;
	//}

	//if (padData.lY < -unresponsive_range)
	//{
	//	isPush[ButtonKind::UpButton] = true;
	//}
	//else if (padData.lY > unresponsive_range)
	//{
	//	isPush[ButtonKind::DownButton] = true;
	//}
	////ボタン判定
	//for (int i = 0; i < 32; i++) {
	//	if (!(padData.rgbButtons[i] & 0x80))
	//	{
	//		continue;
	//	}

	//	switch (i)
	//	{
	//	case 0:
	//		isPush[ButtonKind::Button01] = true;
	//		break;
	//	case 1:
	//		isPush[ButtonKind::Button02] = true;
	//		break;
	//	}
	//}
	////十字キー判定
	//if (padData.rgdwPOV[0] != 0xffffffff)
	//{
	//	switch (padData.rgdwPOV[0])
	//	{
	//		// 上
	//	case 0:
	//		isPush[ButtonKind::UpButton] = true;
	//		break;
	//		// 右上
	//	case 4500:
	//		isPush[ButtonKind::UpButton] = true;
	//		isPush[ButtonKind::RightButton] = true;
	//		break;
	//		// 右
	//	case 9000:
	//		isPush[ButtonKind::RightButton] = true;
	//		break;
	//		// 右下
	//	case 13500:
	//		isPush[ButtonKind::DownButton] = true;
	//		isPush[ButtonKind::RightButton] = true;
	//		break;
	//		// 下
	//	case 18000:
	//		isPush[ButtonKind::DownButton] = true;
	//		break;
	//		// 左下
	//	case 22500:
	//		isPush[ButtonKind::DownButton] = true;
	//		isPush[ButtonKind::LeftButton] = true;
	//		break;
	//		// 左
	//	case 27000:
	//		isPush[ButtonKind::LeftButton] = true;
	//		break;
	//		//左上
	//	case 31500:
	//		isPush[ButtonKind::UpButton] = true;
	//		isPush[ButtonKind::LeftButton] = true;
	//		break;
	//	default:
	//		break;
	//	}
	//}
}

bool Input::PushKey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	if (key[keyNumber]) {
		return true;
	}
	//そうでなければfalseを返す
	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	//指定キーを押しているかつ前回指定キーが押されていなかったらtrueを返す
	if (key[keyNumber] && !preKey[keyNumber]) {
		return true;
	}
	//そうでなければfalseを返す
	return false;
}
