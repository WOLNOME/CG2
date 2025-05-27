#pragma once
#include "BaseScene.h"
#include "Vector2.h"
#include "MyMath.h"
#include "DevelopCamera.h"
#include <vector>

class EvaluationTaskScene : public BaseScene {
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
	///	テキスト描画
	/// </summary>
	void TextDraw() override;

private://メンバ変数
	Input* input_ = nullptr;
	//開発用カメラ
	std::unique_ptr<DevelopCamera> camera;
	Vector3 cameraTranslate = { 0.0f,0.0f,-15.0f };
	Vector3 cameraRotate = { 0.0f,0.0f,0.0f };
	//スカイボックス
	uint32_t textureHandleSkyBox_ = 0u;
	std::unique_ptr<Object3d> skyBox_ = nullptr;

	//パーティクル
	std::vector<std::unique_ptr<Particle>> particles_;

};

