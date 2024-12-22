#pragma once
#include "BaseScene.h"
#include "Input.h"
#include "DevelopCamera.h"
#include "Skydome.h"
#include "Ground.h"
#include "Player.h"
#include <memory>

class GamePlayScene : public BaseScene
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// 終了時
	/// </summary>
	void Finalize() override;
	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;
	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;
private://メンバ変数
	//インプット
	Input* input_ = nullptr;
	//カメラ
	std::unique_ptr<DevelopCamera> camera_ = nullptr;
	//光源系
	std::unique_ptr<DirectionalLight> dLight_ = nullptr;

	//天球
	std::unique_ptr<Skydome> skydome_ = nullptr;
	//地面
	std::unique_ptr<Ground> ground_ = nullptr;


	//プレイヤー
	std::unique_ptr<Player> player_ = nullptr;


};

