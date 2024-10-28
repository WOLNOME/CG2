#pragma once
#include "D3DResourceLeakChecker.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "AudioCommon.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "Camera.h"

class Framework
{
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~Framework() = default;
	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize();
	/// <summary>
	/// 終了時
	/// </summary>
	virtual void Finalize();
	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update();
	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw() = 0;
	/// <summary>
	/// 終了判定
	/// </summary>
	/// <returns></returns>
	virtual bool GetOver() { return isOver; }
public:
	//実行
	void Run();
protected://基盤インスタンス
	D3DResourceLeakChecker leakChecker;
	WinApp* winApp_ = nullptr;
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
	ImGuiManager* imGuiManager_ = nullptr;
	Input* input_ = nullptr;
	AudioCommon* audioCommon_ = nullptr;
	SpriteCommon* spriteCommon_ = nullptr;
	Object3dCommon* object3dCommon_ = nullptr;
	Camera* camera_ = nullptr;
	bool isOver = false;
};

