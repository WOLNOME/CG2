#include "GamePlayScene.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "LineDrawerCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"

void GamePlayScene::Initialize()
{
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//ゲームシーン変数の初期化
	sprite_ = std::make_unique<Sprite>();
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	sprite_->Initialize("Resources/monsterBall.png");
	sprite_->SetAnchorPoint({ 0.5f,0.5f });
	sprite_->SetFlipX(true);

	sprite2_ = std::make_unique<Sprite>();
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	sprite2_->Initialize("Resources/monsterBall.png");
	sprite2Position = { 100.0f,100.0f };
	sprite2_->SetPosition(sprite2Position);
	sprite2_->SetSize({ 300.0f,300.0f });

	obj_ = std::make_unique<Object3d>();
	obj_->Initialize("axis");

	particle_ = std::make_unique<Particle>();
	particle_->Initialize("plane");

	line_ = std::make_unique<LineDrawer>();
	line_->Initialize();

	audio_ = std::make_unique<Audio>();
	audio_->Initialize("Alarm01.wav");

}

void GamePlayScene::Finalize()
{
}

void GamePlayScene::Update()
{
	//LEFTキーを押したら
	if (input_->TriggerKey(DIK_LEFT)) {
		//シーン切り替え依頼
		sceneManager_->SetNextScene("TITLE");
	}

	//モデルの更新
	obj_->Update();
	obj_->SetRotate({ 0.0f,obj_->GetRotate().y + 0.03f,0.0f });

	//パーティクルの更新
	particle_->Update();

	//スプライトの更新
	sprite_->Update();
	sprite_->SetRotation(sprite_->GetRotation() + 0.03f);
	sprite2_->Update();

#ifdef _DEBUG
	ImGui::SetNextWindowSize(ImVec2(500, 100));
	ImGui::Begin("MosterBall");
	ImGui::SliderFloat2("position", &sprite2Position.x, 0.0f, 1200.0f, "%5.1f");
	sprite2_->SetPosition(sprite2Position);
	ImGui::End();

	ImGui::Begin("Audio");
	if (ImGui::Button("PlayAudio")) {
		audio_->Play();
	}
	if (ImGui::Button("StopAudio")) {
		audio_->Stop();
	}
	if (ImGui::Button("PauseAudio")) {
		audio_->Pause();
	}
	if (ImGui::Button("ResumeAudio")) {
		audio_->Resume();
	}
	ImGui::SliderFloat("SetVolume", &volume, 0.0f, 1.0f);
	audio_->SetVolume(volume);

	ImGui::End();

	ImGui::Begin("axis");
	ImGui::SliderFloat3("translate", &translate.x, -10.0f, 10.0f);
	obj_->SetTranslate(translate);
	ImGui::End();

	ImGui::Begin("sphere");
	//球を定義
	Sphere sphere = {
		{0.0f,0.0f,0.0f},
		5.0f
	};
	ImGui::Checkbox("draw", &isDrawSphere_);
	if (isDrawSphere_) {
		//MyMath::DrawSphere(sphere, { 1.0f,0.0f,0.0f,1.0f }, line_.get());
	}
	

	ImGui::End();

#endif // _DEBUG
}

void GamePlayScene::Draw()
{
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	obj_->Draw();

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
	///------------------------------///

	//パーティクルの共通描画設定
	ParticleCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓パーティクル描画開始↓↓↓↓
	///------------------------------///

	particle_->Draw();

	///------------------------------///
	///↑↑↑↑パーティクル描画終了↑↑↑↑
	///------------------------------///


	//線描画共通描画設定
	LineDrawerCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓線描画開始↓↓↓↓
	///------------------------------///

	//線描画
	line_->CreateLine({ 0.0f,1.0f,1.0f }, { 10.0f,2.0f,-4.0f }, { 1.0f,0.0f,0.0f,1.0f });
	line_->CreateLine({ 1.0f,1.0f,1.0f }, { -3.0f,8.0f,3.0f }, { 0.0f,1.0f,0.0f,1.0f });
	line_->Draw();

	///------------------------------///
	///↑↑↑↑線描画終了↑↑↑↑
	///------------------------------///

	//スプライトの共通描画設定
	SpriteCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓スプライト描画開始↓↓↓↓
	///------------------------------///

	//スプライト描画
	sprite_->Draw();
	sprite2_->Draw();


	///------------------------------///
	///↑↑↑↑スプライト描画終了↑↑↑↑
	///------------------------------///

}
