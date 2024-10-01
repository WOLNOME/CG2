#pragma once
#include <wrl.h>

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>


//入力
class Input
{
public:
	//ゲームパッドキーボタンの種類
	enum ButtonKind
	{
		UpButton,
		DownButton,
		LeftButton,
		RightButton,
		Button01,
		Button02,
		ButtonKindMax,
	};

	//ゲームパッドボタンの状態
	enum ButtonState
	{
		ButtonStateNone,
		ButtonStateDown,
		ButtonStatePush,
		ButtonStateUp,
		ButtonStateMax,
	};

public:
	//namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;//エイリアステンプレート

public://メンバ関数
	//初期化
	void Initialize(HINSTANCE wc, HWND hwnd);
	//更新
	void Update();

private://メンバ変数
	//キーボードデバイス
	ComPtr<IDirectInputDevice8> keyboard;

	//ゲームパッドデバイス
	ComPtr<IDirectInputDevice8> gamepad;
};