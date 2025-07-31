#include "DevelopScene.h"
#include "SceneManager.h"
#include <numbers>

void DevelopScene::Initialize() {
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//カメラの生成と初期化
	camera = std::make_unique<DevelopCamera>();
	camera->Initialize();
	camera->worldTransform.rotate = (cameraRotate);
	camera->worldTransform.translate = (cameraTranslate);
	camera->SetFarClip(500.0f);

	//平行光源の生成と初期化
	dirLight = std::make_unique<DirectionalLight>();

	//各光源をシーンライトにセット
	sceneLight_->SetLight(dirLight.get());

	//スカイボックスの生成と初期化
	textureHandleSkyBox_ = TextureManager::GetInstance()->LoadTexture("rostock_laage_airport_4k.dds");
	skyBox_ = std::make_unique<Object3d>();
	skyBox_->Initialize(ShapeTag{}, Shape::ShapeKind::kSkyBox);
	skyBox_->worldTransform.scale = { 300.0f,300.0f,300.0f };

	//3Dオブジェクトの生成と初期化
	teapot_ = std::make_unique<Object3d>();
	teapot_->Initialize(ModelTag{}, "teapot");
	int32_t elthTeapot = TextureManager::GetInstance()->LoadTexture("rostock_laage_airport_4k.dds");
	teapot_->SetEnvironmentLightTextureHandle(elthTeapot);
	teapot_->SetSceneLight(sceneLight_.get());

	terrain_ = std::make_unique<Object3d>();
	terrain_->Initialize(ModelTag{}, "terrain");
	terrain_->worldTransform.translate = { 0.0f,-1.2f,0.0f };
	terrain_->SetSceneLight(sceneLight_.get());

	composite_ = std::make_unique<Object3d>();
	composite_->Initialize(AnimationModelTag{}, "walk", GLTF);
	composite_->worldTransform.translate = { 0.0f,1.0f,-3.0f };
	composite_->worldTransform.rotate = { 0.0f,3.14f,0.0f };
	composite_->SetEnvironmentLightTextureHandle(elthTeapot);
	composite_->SetSceneLight(sceneLight_.get());
	composite_->SetNewAnimation("walk", "walk");
	composite_->SetNewAnimation("sneakWalk", "sneakWalk");
	composite_->SetCurrentAnimation("walk");

	//パーティクルの生成・初期化
	ParticleManager::GetInstance()->SetCamera(camera.get());
	particle_ = std::make_unique<Particle>();
	particle_->Initialize("gpu", "gpu");
	particle_->emitter_.transform.translate = { 0.0f,0.0f,30.0f };
	particle_->emitter_.transform.scale = { 10.0f,10.0f,1.0f };

}

void DevelopScene::Finalize() {
}

void DevelopScene::Update() {
	//シーン共通の更新
	BaseScene::Update();

	//TABでシーン再読み込み
	if (input_->TriggerKey(DIK_TAB)) {
		sceneManager_->SetNextScene("DEVELOP");
	}

	//カメラの更新
	camera->Update();

	//スカイボックスの更新
	skyBox_->Update();

	//ティーポットの回転
	teapot_->worldTransform.rotate.y += 0.03f;
	//オブジェクトの更新
	teapot_->Update();
	terrain_->Update();
	composite_->Update();

#ifdef _DEBUG
	
	//平行光源のデバッグ用ImGui
	dirLight->DebugWithImGui(L"1");

	ImGui::Begin("複合アニメーション");
	//選択肢
	const char* items[] = { "walk","sneakWalk" };
	static int currentItem = 0;
	if (ImGui::Combo("アニメーションのアイテム", &currentItem, items, IM_ARRAYSIZE(items))) {
		composite_->SetCurrentAnimation(items[currentItem]);
	}
	ImGui::End();

#endif // _DEBUG
}

void DevelopScene::Draw() {
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	//スカイボックス描画
	skyBox_->Draw(camera.get(), textureHandleSkyBox_);

	terrain_->Draw(camera.get());
	teapot_->Draw(camera.get());

	composite_->Draw(camera.get());

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
	///------------------------------///


	//線描画共通描画設定
	LineDrawerCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓線描画開始↓↓↓↓
	///------------------------------///

	///------------------------------///
	///↑↑↑↑線描画終了↑↑↑↑
	///------------------------------///

	//スプライトの共通描画設定
	SpriteCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓スプライト描画開始↓↓↓↓
	///------------------------------///

	///------------------------------///
	///↑↑↑↑スプライト描画終了↑↑↑↑
	///------------------------------///



}
