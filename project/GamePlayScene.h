#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Audio.h"
#include "Vector2.h"

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
	Sprite* sprite_ = nullptr;
	Sprite* sprite2_ = nullptr;
	Vector2 sprite2Position;
	Object3d* object3d_ = nullptr;
	Object3d* object3d2_ = nullptr;
	Audio* audio_ = nullptr;
};

