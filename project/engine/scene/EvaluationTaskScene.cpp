#include "EvaluationTaskScene.h"
#include "SceneManager.h"
#include <numbers>

void EvaluationTaskScene::Initialize() {
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//カメラの生成と初期化
	camera = std::make_unique<DevelopCamera>();
	camera->Initialize();
	camera->SetRotate({ cameraRotate });
	camera->SetTranslate(cameraTranslate);
	camera->SetFarClip(500.0f);

	//スカイボックスの生成と初期化
	textureHandleSkyBox_ = TextureManager::GetInstance()->LoadTexture("earth-cubemap.dds");
	skyBox_ = std::make_unique<Object3d>();
	skyBox_->Initialize(ShapeTag{}, Shape::ShapeKind::kSkyBox);
	skyBox_->worldTransform.scale = { 300.0f,300.0f,300.0f };

	ParticleManager::GetInstance()->SetCamera(camera.get());
	//各パーティクルの生成

}

void EvaluationTaskScene::Finalize() {
}

void EvaluationTaskScene::Update() {
	//シーン共通の更新
	BaseScene::Update();

	//TABでシーン再読み込み
	if (input_->TriggerKey(DIK_TAB)) {
		sceneManager_->SetNextScene("EVALUATIONTASK");
	}

	//カメラの更新
	camera->Update();

	//スカイボックスの更新
	skyBox_->Update();

#ifdef _DEBUG

#endif // _DEBUG
}

void EvaluationTaskScene::Draw() {
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	//スカイボックス描画
	skyBox_->Draw(camera.get(), textureHandleSkyBox_);

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

	////スプライト描画
	//sprite_->Draw();
	//sprite2_->Draw();


	///------------------------------///
	///↑↑↑↑スプライト描画終了↑↑↑↑
	///------------------------------///
}

void EvaluationTaskScene::TextDraw() {
	///------------------------------///
	///↑↑↑↑テキスト描画終了↑↑↑↑
	///------------------------------///

	///------------------------------///
	///↑↑↑↑テキスト描画終了↑↑↑↑
	///------------------------------///
}
