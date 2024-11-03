#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "imgui.h"

class ImGuiManager
{
private://コンストラクタ等の隠蔽
	static ImGuiManager* instance;

	ImGuiManager() = default;//コンストラクタ隠蔽
	~ImGuiManager() = default;//デストラクタ隠蔽
	ImGuiManager(ImGuiManager&) = delete;//コピーコンストラクタ封印
	ImGuiManager& operator=(ImGuiManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static ImGuiManager* GetInstance();
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

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

private://メンバ変数
	
};

