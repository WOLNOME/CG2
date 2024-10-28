#include "MyGame.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Model.h"
#include "SpriteCommon.h"
#include "ModelCommon.h"

void MyGame::Initialize()
{
	//ゲーム基盤部の初期化
	Framework::Initialize();

	//ゲームシーン変数の初期化
	sprite_ = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	sprite_->Initialize("Resources/uvChecker.png");
	sprite_->SetAnchorPoint({ 0.5f,0.5f });
	sprite_->SetFlipX(true);

	sprite2_ = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	sprite2_->Initialize("Resources/monsterBall.png");
	sprite2Position = { 100.0f,100.0f };
	sprite2_->SetPosition(sprite2Position);
	sprite2_->SetSize({ 300.0f,300.0f });

	ModelManager::GetInstance()->LoadModel("plane.obj");
	model_ = new Model();
	model_ = ModelManager::GetInstance()->FindModel("plane.obj");

	ModelManager::GetInstance()->LoadModel("axis.obj");
	model2_ = new Model();
	model2_= ModelManager::GetInstance()->FindModel("axis.obj");

	Audio::GetInstance()->LoadWave("Alarm01.wav");

}

void MyGame::Finalize()
{

	delete model2_;
	delete model_;
	delete sprite2_;
	delete sprite_;

	//ゲーム基盤解放
	Framework::Finalize();
}

void MyGame::Update()
{
	//ゲーム基盤更新
	Framework::Update();

	//ImGui受付開始
	imGuiManager_->Begin();

	///==============================///
	///          更新処理
	///==============================///

	//モデルの更新
	model_->Update();
	model2_->Update();

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
		Audio::GetInstance()->Play();
	}
	ImGui::End();

#endif // _DEBUG
	//ImGuiの内部コマンドを生成する
	imGuiManager_->End();

}

void MyGame::Draw()
{
	///==============================///
	///          描画処理
	///==============================///

		//描画前処理
	dxCommon_->PreDraw();
	srvManager_->PreDraw();

	//3Dモデルの共通描画設定
	ModelCommon::GetInstance()->SettingCommonDrawing();
	ModelCommon::GetInstance()->Draw();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	model_->Draw();
	model2_->Draw();

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
	
	//ImGuiの描画
	imGuiManager_->Draw();

	//描画後処理
	dxCommon_->PostDraw();
}

