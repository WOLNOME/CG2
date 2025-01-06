#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Audio.h"
#include "Vector2.h"
#include "Input.h"
#include "LineDrawer.h"
#include <cstdint>
#include <memory>

class TitleScene : public BaseScene
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

	//タイトルスプライト
	uint32_t textureHandleTitle_ = 0u;
	std::unique_ptr<Sprite> spriteTitle_ = nullptr;
};

