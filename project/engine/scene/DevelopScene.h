#pragma once
#include "BaseScene.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Skybox.h"
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
	std::unique_ptr<DirectionalLight> dirLight;
	//点光源
	std::unique_ptr<PointLight> pointLight;
	std::unique_ptr<LineDrawer> plMark;
	bool isDrawPLMark = false;
	std::unique_ptr<PointLight> pointLight2;
	std::unique_ptr<LineDrawer> plMark2;
	bool isDrawPLMark2 = false;
	//スポットライト
	std::unique_ptr<SpotLight> spotLight;
	std::unique_ptr<LineDrawer> slMark;
	bool isDrawSLMark = false;
	//スカイボックス
	WorldTransform wtSkybox_;
	uint32_t textureHandleSkybox_ = UINT32_MAX;
	std::unique_ptr<Skybox> skybox_ = nullptr;

	uint32_t textureHandleSprite_ = UINT32_MAX;
	std::unique_ptr<Sprite> sprite_ = nullptr;
	uint32_t textureHandleSprite2_ = UINT32_MAX;
	std::unique_ptr<Sprite> sprite2_ = nullptr;
	Vector2 sprite2Position;

	WorldTransform wtAxis_;
	std::unique_ptr<Object3d> axis_ = nullptr;

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

	Particle::Emitter emitter_;
	Particle::AccelerationField field_;
	std::unique_ptr<Particle> particle_ = nullptr;

	std::unique_ptr<LineDrawer> line_ = nullptr;

	bool isDrawSphere_ = false;
	std::unique_ptr<Audio> audio_ = nullptr;
	float volume = 0.5f;
};

