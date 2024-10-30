#pragma once
#include "Sprite.h"
#include "Object3d.h"
#include "Audio.h"
#include "Framework.h"
#include "GamePlayScene.h"
#include <memory>

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
	
private://シーン
	std::unique_ptr<GamePlayScene> gamePlayScene_ = nullptr;
	
};

