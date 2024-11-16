#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Particle.h"
#include "LineDrawer.h"
#include "Audio.h"
#include "Vector2.h"
#include "Input.h"
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
	Input* input_ = nullptr;

	std::unique_ptr<Sprite> sprite_ = nullptr;
	std::unique_ptr<Sprite> sprite2_ = nullptr;
	std::unique_ptr<Object3d> obj_ = nullptr;
	std::unique_ptr<Particle> particle_ = nullptr;
	std::unique_ptr<LineDrawer> line_ = nullptr;
	Vector2 sprite2Position;
	std::unique_ptr<Audio> audio_ = nullptr;
	float volume = 0.5f;

	Vector3 translate = { 0.0f,0.0f,0.0f };
};

