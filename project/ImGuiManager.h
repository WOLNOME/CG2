#pragma once
#include <wrl.h>
#include <d3d12.h>

class DirectXCommon;
class WinApp;
class SrvManager;
class ImGuiManager
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, WinApp* winApp, SrvManager* srvManager);

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	/// <summary>
	/// ImGui受付開始
	/// </summary>
	void Begin();

	/// <summary>
	/// ImGui受付終了
	/// </summary>
	void End();

	/// <summary>
	/// 画面への描画
	/// </summary>
	void Draw();

private://インスタンス
	DirectXCommon* dxCommon_ = nullptr;
	WinApp* winApp_ = nullptr;
	SrvManager* srvManager_ = nullptr;
private://メンバ変数
	
};

