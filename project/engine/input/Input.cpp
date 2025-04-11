#include "Input.h"
#include <cassert>
#include <Windows.h>
#include "WinApp.h"
#include "Logger.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

Input* Input::instance = nullptr;

Input* Input::GetInstance()
{
	if (instance == nullptr) {
		instance = new Input;
	}
	return instance;
}

void Input::Initialize()
{
	//ダイレクトインプットの初期化
	InitDirectInput();
	//マウスの生成
	GenerateMouse();
	//キーボードの生成
	GenerateKeyboard();
	//ゲームパッドの生成
	GenerateGamepad();

	padData;
}

void Input::Update()
{
	HRESULT hr;

	// マウスの状態取得
	preMouseData = mouseData;
	if (mouse) {
		hr = mouse->Acquire();
		if (SUCCEEDED(hr)) {
			hr = mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseData);
			if (FAILED(hr)) {
				mouse->Unacquire();
				mouse.Reset();
				mouse = nullptr;
			}
		}
		else {
			mouse->Unacquire();
			mouse.Reset();
			mouse = nullptr;
		}
	}

	//キーボードの前回の入力を保存
	memcpy(preKey, key, sizeof(key));
	//キーボード再接続
	if (!keyboard) {
		GenerateKeyboard();
	}
	//キーボードの入力状態取得
	if (keyboard) {
		//グラフィックスコマンド
		hr = keyboard->Acquire();
		if (SUCCEEDED(hr)) {
			//全キーの入力状態を取得する
			hr = keyboard->GetDeviceState(sizeof(key), key);
			if (FAILED(hr)) {
				// ここで GetDeviceState が失敗する場合、キーボードが切断された可能性あり
				keyboard->Unacquire();
				keyboard.Reset();
				keyboard = nullptr;
			}
		}
		else {
			// Acquire に失敗した場合、デバイスが切断された可能性あり
			keyboard->Unacquire();
			keyboard.Reset();
			keyboard = nullptr;
		}
	}

	//前回のゲームパッド状態を保存
	prePadData = padData;

	//ゲームパッド再接続
	if (!gamepad) {
		GenerateGamepad();
	}
	//ゲームパッドの入力状態取得
	if (gamepad) {
		//ゲームパッドのグラフィックスコマンド
		hr = gamepad->Acquire();
		if (SUCCEEDED(hr)) {
			//Acquireが通ったらPollを呼び出す
			hr = gamepad->Poll();
			if (SUCCEEDED(hr)) {
				//デバイス取得
				hr = gamepad->GetDeviceState(sizeof(DIJOYSTATE), &padData);
			}
		}
		//Acquireに失敗→途中でゲームパッドの接続が切断されたと判断
		else {
			//パッドデータの削除
			ZeroMemory(&padData, sizeof(DIJOYSTATE));
			//gamepadの解放
			gamepad->Unacquire();
			gamepad.Reset();
			gamepad = nullptr;
		}
	}
}

void Input::Finalize()
{
	delete instance;
	instance = nullptr;
}

void Input::InitDirectInput()
{
	HRESULT hr;
	//DirectInputの初期化
	hr = DirectInput8Create(
		WinApp::GetInstance()->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));
}

void Input::GenerateKeyboard()
{
	HRESULT hr;

	//キーボードデバイスの生成
	keyboard = nullptr;
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));
	//入力データ形式のセット
	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));
	//ウィンドウがないためスキップ
	if (!IsWindow(WinApp::GetInstance()->GetHwnd())) {
		Logger::Log("ウィンドウ消えてるからSetCooperativeLevelスキップ");
		return;
	}
	//排他制御レベルのセット
	hr = keyboard->SetCooperativeLevel(
		WinApp::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));
}

void Input::GenerateMouse()
{
	HRESULT hr;

	// マウスデバイスの生成
	mouse = nullptr;
	hr = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
	if (FAILED(hr)) {
		// マウスが接続されていない場合、処理をスキップ
		mouse = nullptr;
	}
	else {
		// マウスが見つかった場合のみ初期化処理を行う
		hr = mouse->SetDataFormat(&c_dfDIMouse);
		assert(SUCCEEDED(hr));

		// 協調モードの設定
		hr = mouse->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		assert(SUCCEEDED(hr));
	}
}

void Input::GenerateGamepad()
{
	HRESULT hr;

	//ゲームパッドデバイスの生成
	gamepad = nullptr;
	hr = directInput->CreateDevice(GUID_Joystick, &gamepad, NULL);
	if (FAILED(hr)) {
		// ゲームパッドが接続されていない場合、ゲームパッドなしで進める
		gamepad = nullptr;
	}
	else {
		// ゲームパッドが見つかった場合のみ初期化処理を行う
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
		hr = gamepad->SetProperty(DIPROP_AXISMODE, &diprop.diph);
		assert(SUCCEEDED(hr));

		// X軸の値の範囲設定
		DIPROPRANGE diprg;
		ZeroMemory(&diprg, sizeof(diprg));
		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.diph.dwObj = DIJOFS_X;
		diprg.lMin = -1000;
		diprg.lMax = 1000;
		hr = gamepad->SetProperty(DIPROP_RANGE, &diprg.diph);
		assert(SUCCEEDED(hr));

		// Y軸の値の範囲設定
		diprg.diph.dwObj = DIJOFS_Y;
		hr = gamepad->SetProperty(DIPROP_RANGE, &diprg.diph);
		assert(SUCCEEDED(hr));

		// 右スティックのX軸範囲設定
		DIPROPRANGE diprgRightX;
		ZeroMemory(&diprgRightX, sizeof(diprgRightX));
		diprgRightX.diph.dwSize = sizeof(diprgRightX);
		diprgRightX.diph.dwHeaderSize = sizeof(diprgRightX.diph);
		diprgRightX.diph.dwHow = DIPH_BYOFFSET;
		diprgRightX.diph.dwObj = DIJOFS_RX;  // 右スティックのX軸
		diprgRightX.lMin = -1000;
		diprgRightX.lMax = 1000;
		assert(SUCCEEDED(gamepad->SetProperty(DIPROP_RANGE, &diprgRightX.diph)));

		// 右スティックのY軸範囲設定
		DIPROPRANGE diprgRightY;
		ZeroMemory(&diprgRightY, sizeof(diprgRightY));
		diprgRightY.diph.dwSize = sizeof(diprgRightY);
		diprgRightY.diph.dwHeaderSize = sizeof(diprgRightY.diph);
		diprgRightY.diph.dwHow = DIPH_BYOFFSET;
		diprgRightY.diph.dwObj = DIJOFS_RY;  // 右スティックのY軸
		diprgRightY.lMin = -1000;
		diprgRightY.lMax = 1000;
		assert(SUCCEEDED(gamepad->SetProperty(DIPROP_RANGE, &diprgRightY.diph)));

		// 協調モードの設定
		hr = gamepad->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_EXCLUSIVE | DISCL_FOREGROUND);
		assert(SUCCEEDED(hr));
	}
}

bool Input::PushMouseButton(MouseButton button)
{
	// マウスボタンが押されているか確認
	switch (button) {
	case LeftButton:
		return (mouseData.rgbButtons[0] & 0x80) != 0;
	case RightButton:
		return (mouseData.rgbButtons[1] & 0x80) != 0;
	case MiddleButton:
		return (mouseData.rgbButtons[2] & 0x80) != 0;
	default:
		return false;
	}
}

bool Input::TriggerMouseButton(MouseButton button)
{
	// マウスボタンが押された瞬間か確認
	switch (button) {
	case LeftButton:
		return (mouseData.rgbButtons[0] & 0x80) != 0 && !(preMouseData.rgbButtons[0] & 0x80);
	case RightButton:
		return (mouseData.rgbButtons[1] & 0x80) != 0 && !(preMouseData.rgbButtons[1] & 0x80);
	case MiddleButton:
		return (mouseData.rgbButtons[2] & 0x80) != 0 && !(preMouseData.rgbButtons[2] & 0x80);
	default:
		return false;
	}
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

bool Input::PushPadButton(GamepadButton button)
{
	// 十字キーの処理
	switch (button) {
	case DPadUp:
		return padData.rgdwPOV[0] == 0;
	case DPadRight:
		return padData.rgdwPOV[0] == 9000;
	case DPadDown:
		return padData.rgdwPOV[0] == 18000;
	case DPadLeft:
		return padData.rgdwPOV[0] == 27000;
	default:
		break;
	}

	// 通常のボタンの処理
	if (button >= 0 && button < 16) {
		return (padData.rgbButtons[button] & 0x80) != 0;
	}

	return false;
}

bool Input::TriggerPadButton(GamepadButton button)
{
	// 十字キーの処理
	switch (button) {
	case DPadUp:
		return (padData.rgdwPOV[0] == 0 && prePadData.rgdwPOV[0] != 0);
	case DPadRight:
		return (padData.rgdwPOV[0] == 9000 && prePadData.rgdwPOV[0] != 9000);
	case DPadDown:
		return (padData.rgdwPOV[0] == 18000 && prePadData.rgdwPOV[0] != 18000);
	case DPadLeft:
		return (padData.rgdwPOV[0] == 27000 && prePadData.rgdwPOV[0] != 27000);
	default:
		break;
	}

	// 通常のボタンの処理
	if (button >= 0 && button < 16) {
		return (padData.rgbButtons[button] & 0x80) != 0 && !(prePadData.rgbButtons[button] & 0x80);
	}

	return false;
}

Vector2 Input::GetMousePosition()
{
	return Vector2(static_cast<float>(mouseData.lX), static_cast<float>(mouseData.lY));
}

Vector2 Input::GetMouseDelta()
{
	return Vector2(static_cast<float>(mouseData.lX - preMouseData.lX), static_cast<float>(mouseData.lY - preMouseData.lY));
}

float Input::GetMouseScrollCount()
{
	// 1回のスクロールは120単位
	const float scrollPerUnit = 120.0f;

	// lZはマウスホイールの変化量（単位は120ごと）
	return static_cast<float>(mouseData.lZ) / scrollPerUnit;
}

Vector2 Input::GetLeftStickDir()
{
	const int deadZone = 200; // デッドゾーンの設定
	int x = padData.lX;
	int y = padData.lY;

	// デッドゾーン内の値は 0 にする
	if (abs(x) < deadZone) x = 0;
	if (abs(y) < deadZone) y = 0;

	// 正規化してベクトルを返す
	Vector2 dir(static_cast<float>(x) / 1000.0f, static_cast<float>(y) / 1000.0f);
	return dir.Normalize();
}

Vector2 Input::GetRightStickDir()
{
	const int deadZone = 200; // デッドゾーンの設定
	int x = padData.lRx;
	int y = padData.lRy;

	// デッドゾーン内の値は 0 にする
	if (abs(x) < deadZone) x = 0;
	if (abs(y) < deadZone) y = 0;

	// 正規化してベクトルを返す
	Vector2 dir(static_cast<float>(x) / 1000.0f, static_cast<float>(y) / 1000.0f);
	return dir.Normalize();
}