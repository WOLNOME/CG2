#pragma once
#include "BaseScene.h"
class ClearScene : public BaseScene
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
	/// <summary>
	/// テキスト描画
	/// </summary>
	void TextDraw() override;
private://メンバ変数
	//インプット
	Input* input_ = nullptr;

	//スプライト
	uint32_t textureHandleClear_ = 0u;
	std::unique_ptr<Sprite> spriteClear_ = nullptr;

	uint32_t textureHandleUI_SPACE_ = 0u;
	std::unique_ptr<Sprite> spriteUI_SPACE_ = nullptr;

};

