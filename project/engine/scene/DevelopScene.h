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
	//スプライト
	uint32_t textureHandleSprite_ = 0u;
	std::unique_ptr<Sprite> sprite_ = nullptr;
	uint32_t textureHandleSprite2_ = 0u;
	std::unique_ptr<Sprite> sprite2_ = nullptr;
	Vector2 sprite2Position;
	//スカイボックス
	uint32_t textureHandleSkyBox_ = 0u;
	std::unique_ptr<Object3d> skyBox_ = nullptr;

	//3Dオブジェクト
	std::unique_ptr<Object3d> teapot_ = nullptr;

	std::unique_ptr<Object3d> terrain_ = nullptr;

	std::unique_ptr<Object3d> animatedCube_ = nullptr;

	std::unique_ptr<Object3d> sneakWalk_ = nullptr;

	std::unique_ptr<Object3d> walk_ = nullptr;

	std::unique_ptr<Object3d> composite_ = nullptr;

	std::unique_ptr<Object3d> simpleSkin_ = nullptr;

	//パーティクル
	std::unique_ptr<Particle> particle_ = nullptr;

	//線
	std::unique_ptr<LineDrawer> line_ = nullptr;

	//オーディオ
	bool isDrawSphere_ = false;
	std::unique_ptr<Audio> audio_ = nullptr;
	float volume = 0.5f;

	//テキスト
	int timer_ = 0;
	std::unique_ptr<TextWrite> text_;
};

