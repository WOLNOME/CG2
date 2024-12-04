#pragma once
#include <windows.h>
#include <wrl.h>
#include <cstdint>

class WinApp
{
private://コンストラクタ等の隠蔽
	static WinApp* instance;

	WinApp() = default;//コンストラクタ隠蔽
	~WinApp() = default;//デストラクタ隠蔽
	WinApp(WinApp&) = delete;//コピーコンストラクタ封印
	WinApp& operator=(WinApp&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static WinApp* GetInstance();
public://静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
public://定数(公開は定数のみ)
	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public:
	void Initialize();
	void Finalize();
	//メッセージ処理
	bool ProcessMessage();

public://ゲッター
	HWND GetHwnd()const { return hwnd; }
	HINSTANCE GetHInstance()const { return wc.hInstance; }
private:
	//ウィンドウハンドル
	HWND hwnd = nullptr;
	//ウィンドウクラスの設定
	WNDCLASS wc{};

};

