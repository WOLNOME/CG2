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
private://コンストラクタ等の隠蔽
	static Input* instance;

	Input() = default;//コンストラクタ隠蔽
	~Input() = default;//デストラクタ隠蔽
	Input(Input&) = delete;//コピーコンストラクタ封印
	Input& operator=(Input&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static Input* GetInstance();
public:
	//namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;//エイリアステンプレート

public://メンバ関数
	//初期化
	void Initialize();
	//更新
	void Update();
	//終了
	void Finalize();

public://固有の処理
	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);

private://インスタンス

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