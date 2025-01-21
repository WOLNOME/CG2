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
	//平行光源
	std::unique_ptr<DirectionalLight> dirLight_;
	//ポイントライト
	std::unique_ptr<PointLight> pointLight1_ = nullptr;
	std::unique_ptr<PointLight> pointLight2_ = nullptr;
	Vector3 pl1Velocity_ = { 0.0f,0.0f,0.0f };
	Vector3 pl2Velocity_ = { 0.0f,0.0f,0.0f };
	//スポットライト
	std::unique_ptr<SpotLight> spotLight1_ = nullptr;
	std::unique_ptr<SpotLight> spotLight2_ = nullptr;

	//UV球
	WorldTransform wtSphere_;
	uint32_t thSphere_ = 0u;
	std::unique_ptr<Object3d> sphere_ = nullptr;
	//plane.obj
	WorldTransform wtPlaneObj_;
	std::unique_ptr<Object3d> planeObj_ = nullptr;
	//plane.glTF
	WorldTransform wtPlaneGltf_;
	std::unique_ptr<Object3d> planeGltf_ = nullptr;


	WorldTransform wtTerrain_;
	std::unique_ptr<Object3d> terrain_ = nullptr;

	WorldTransform wtAnimatedCube_;
	std::unique_ptr<Object3d> animatedCube_ = nullptr;

	WorldTransform wtSneakWalk_;
	std::unique_ptr<Object3d> sneakWalk_ = nullptr;

	WorldTransform wtWalk_;
	std::unique_ptr<Object3d> walk_ = nullptr;

	WorldTransform wtSimpleSkin_;
	std::unique_ptr<Object3d> simpleSkin_ = nullptr;

	std::unique_ptr<LineDrawer> line_ = nullptr;

	bool isDrawSphere_ = false;
	std::unique_ptr<Audio> audio_ = nullptr;
	float volume = 0.5f;
};

