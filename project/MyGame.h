#pragma once
#include "Framework.h"
#include "Sprite.h"
#include "Model.h"
#include "Audio.h"

class MyGame : public Framework
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
private://基盤インスタンス
	
private://シーンの持つメンバ変数
	Sprite* sprite_ = nullptr;
	Sprite* sprite2_ = nullptr;
	Vector2 sprite2Position;
	Model* model_ = nullptr;
	Model* model2_ = nullptr;
	Audio* audio_ = nullptr;
};

