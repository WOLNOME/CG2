#pragma once
#include "Sprite.h"
#include "Object3d.h"
#include "Audio.h"
#include "Vector2.h"

class GamePlayScene
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
private://メンバ変数
	Sprite* sprite_ = nullptr;
	Sprite* sprite2_ = nullptr;
	Vector2 sprite2Position;
	Object3d* object3d_ = nullptr;
	Object3d* object3d2_ = nullptr;
	Audio* audio_ = nullptr;
};

