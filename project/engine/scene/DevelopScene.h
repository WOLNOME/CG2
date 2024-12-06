#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Particle.h"
#include "LineDrawer.h"
#include "Audio.h"
#include "Vector2.h"
#include "Input.h"
#include "MyMath.h"
#include "WorldTransform.h"
#include "DevelopCamera.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include <memory>
#include <cstdint>

class DevelopScene : public BaseScene
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
	//開発用カメラ
	std::unique_ptr<DevelopCamera> camera;
	Vector3 cameraTranslate = { 0.0f,0.0f,-15.0f };
	Vector3 cameraRotate = { 0.0f,0.0f,0.0f };
	
	//パーティクル1
	std::unique_ptr<Particle> particle_ = nullptr;
	Emitter emitter1_;
	AccelerationField field1_;
	bool isDisplay1_ = true;

	//いたポリ
	std::unique_ptr<Particle> itapori_ = nullptr;
	Emitter emitter2_;
	AccelerationField field2_;
	bool isDisplay2_ = true;
};

