#include "GamePlayScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"

void GamePlayScene::Initialize()
{
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//ゲームシーン変数の初期化
	sprite_ = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	sprite_->Initialize("Resources/monsterBall.png");
	sprite_->SetAnchorPoint({ 0.5f,0.5f });
	sprite_->SetFlipX(true);

	sprite2_ = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	sprite2_->Initialize("Resources/monsterBall.png");
	sprite2Position = { 100.0f,100.0f };
	sprite2_->SetPosition(sprite2Position);
	sprite2_->SetSize({ 300.0f,300.0f });

	audio_ = new Audio();
	audio_->Initialize("Alarm01.wav");

}

void GamePlayScene::Finalize()
{
	delete audio_;
	delete sprite2_;
	delete sprite_;
}

void GamePlayScene::Update()
{
	//LEFTキーを押したら
	if (input_->TriggerKey(DIK_LEFT)) {
		//シーン切り替え依頼
		sceneManager_->SetNextScene("TITLE");
	}

	//モデルの更新

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


	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
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
