#pragma once
#include <wrl.h>

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

///ゲームパッドのコマンド
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
//入力
class Input
{
public:
	//namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;//エイリアステンプレート

public://メンバ関数
	//初期化
	void Initialize(HINSTANCE wc, HWND hwnd);
	//更新
	void Update();

public://固有の処理
	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);


private://メンバ変数
	//キーボードデバイス
	ComPtr<IDirectInputDevice8> keyboard;
	//全キーの状態
	BYTE key[256] = {};
	//前回の全キーの状態
	BYTE preKey[256] = {};

	//ゲームパッドデバイス
	ComPtr<IDirectInputDevice8> gamepad;


};