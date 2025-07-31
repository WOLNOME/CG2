#pragma once
#include "BaseScene.h"
#include "Vector2.h"
#include "MyMath.h"
#include "DevelopCamera.h"

class DevelopScene : public BaseScene {
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
	//開発用カメラ
	std::unique_ptr<DevelopCamera> camera;
	Vector3 cameraTranslate = { 0.0f,0.0f,-15.0f };
	Vector3 cameraRotate = { 0.0f,0.0f,0.0f };
	//平行光源
	std::unique_ptr<DirectionalLight> dirLight;
	//スカイボックス
	uint32_t textureHandleSkyBox_ = 0u;
	std::unique_ptr<Object3d> skyBox_ = nullptr;

	//3Dオブジェクト
	std::unique_ptr<Object3d> teapot_ = nullptr;

	std::unique_ptr<Object3d> terrain_ = nullptr;
	
	std::unique_ptr<Object3d> composite_ = nullptr;

	//パーティクル
	std::unique_ptr<Particle> particle_ = nullptr;

};

