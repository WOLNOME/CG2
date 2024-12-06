#include "DevelopScene.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "LineDrawerCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"
#include <numbers>

void DevelopScene::Initialize()
{
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//カメラの生成と初期化
	camera = std::make_unique<DevelopCamera>();
	camera->Initialize();
	camera->SetRotate({ cameraRotate });
	camera->SetTranslate(cameraTranslate);
	camera->SetFarClip(100.0f);

	//パーティクル1
	particle_ = std::make_unique<Particle>();
	particle_->Initialize("circle");
	emitter1_.transform.scale = { 1.0f,1.0f,1.0f };
	emitter1_.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter1_.transform.translate = { 0.0f,-5.0f,-5.0f };
	emitter1_.count = 3;
	emitter1_.frequency = 0.3f;
	emitter1_.frequencyTime = 0.0f;
	emitter1_.isInfinity = false;
	field1_.acceleration = { 0.0f,15.0f,0.0f };
	field1_.area = {
		{-10.0f,-10.0f,-10.0f},
		{10.0f,10.0f,10.0f}
	};
	//いたポリ
	itapori_ = std::make_unique<Particle>();
	itapori_->Initialize("plane",true);
	emitter2_.transform.scale = { 1.0f,1.0f,1.0f };
	emitter2_.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter2_.transform.translate = { 0.0f,0.0f,0.0f };
	emitter2_.count = 10;
	emitter2_.frequency = 0.3f;
	emitter2_.frequencyTime = 0.0f;
	emitter2_.isInfinity = true;
	field2_.acceleration = { 0.0f,0.0f,0.0f };
	field2_.area = {
		{0.0f,0.0f,0.0f},
		{1.0f,1.0f,1.0f}
	};

	//いたポリ生成
	itapori_->AddParticle(emitter2_);



}

void DevelopScene::Finalize()
{
}

void DevelopScene::Update()
{
	//カメラの更新
	camera->Update();

	//シーンライトの更新処理
	sceneLight_->Update(camera.get());



#ifdef _DEBUG

	ImGui::Begin("particle");
	ImGui::SliderFloat3("EmitterTranslate", &emitter1_.transform.translate.x, -10.0f, 10.0f);
	ImGui::SliderInt("EmitterCount", &emitter1_.count, 0, 4);
	ImGui::SliderFloat("EmitterFrequency", &emitter1_.frequency, 0.1f, 0.5f);
	ImGui::Checkbox("isDisplay", &isDisplay1_);
	if(ImGui::Button("AddParticle")) {
		particle_->AddParticle(emitter1_);
	}
	ImGui::End();


	ImGui::Begin("itapori");
	ImGui::Checkbox("isDisplay", &isDisplay2_);
	ImGui::End();

#endif // _DEBUG
}

void DevelopScene::Draw()
{
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
	///------------------------------///

	//パーティクルの共通描画設定
	ParticleCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓パーティクル描画開始↓↓↓↓
	///------------------------------///

	if (isDisplay1_) {
		particle_->Draw(*camera.get(), emitter1_, field1_);
	}
	if (isDisplay2_) {
		itapori_->Draw(*camera.get(), emitter2_, field2_);
	}

	///------------------------------///
	///↑↑↑↑パーティクル描画終了↑↑↑↑
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
