#include "TitleScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"

void TitleScene::Initialize()
{
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//ゲームシーン変数の初期化
	sprite_ = std::make_unique<Sprite>();
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	sprite_->Initialize("Resources/uvChecker.png");
	sprite_->SetAnchorPoint({ 0.5f,0.5f });
	sprite_->SetFlipX(true);

	sprite2_ = std::make_unique<Sprite>();
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	sprite2_->Initialize("Resources/uvChecker.png");
	sprite2Position = { 100.0f,100.0f };
	sprite2_->SetPosition(sprite2Position);
	sprite2_->SetSize({ 300.0f,300.0f });

	audio_ = std::make_unique<Audio>();
	audio_->Initialize("Alarm01.wav");
}

void TitleScene::Finalize()
{
	
}

void TitleScene::Update()
{
	//RIGHTキーを押したら
	if (input_->TriggerKey(DIK_RIGHT)) {
		//シーン切り替え依頼
		sceneManager_->SetNextScene("GAMEPLAY");
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

	//buttonを繰り上げたりする
	if (input_->TriggerKey(DIK_UP)) {
		button++;
	}
	if (input_->TriggerKey(DIK_DOWN)) {
		button--;
	}
	//imguiでbuttonの値を確認
	ImGui::Begin("button");
	ImGui::Text("num : %d", button);
	ImGui::End();
	//buttonに応じたパッドボタンが押されているかの確認処理
	if (input_->PushButton((GamepadButton)button)) {
		ImGui::Begin("get");
		ImGui::End();
	}

	//Lスティックの傾き量でスプライトを移動
	sprite_->SetPosition(sprite_->GetPosition() + (input_->GetRightStickDir()*5));



#endif // _DEBUG
}

void TitleScene::Draw()
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
