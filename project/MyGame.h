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
#include "Sprite.h"
#include "Object3d.h"
#include "Audio.h"

class MyGame
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 終了時
	/// </summary>
	void Finalize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	/// <summary>
	/// 終了判定
	/// </summary>
	/// <returns></returns>
	bool GetOver() { return isOver; }
private:
	/// <summary>
	/// 基盤部の初期化
	/// </summary>
	void BaseInitialize();
private://基盤インスタンス
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
private://シーンの持つメンバ変数
	Sprite* sprite_ = nullptr;
	Sprite* sprite2_ = nullptr;
	Vector2 sprite2Position;
	Object3d* object3d_ = nullptr;
	Object3d* object3d2_ = nullptr;
	Audio* audio_ = nullptr;
};

