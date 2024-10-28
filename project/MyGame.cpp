#include "MyGame.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Model.h"

void MyGame::Initialize()
{
	//ゲーム基盤部の初期化
	Framework::Initialize();

	//ゲームシーン変数の初期化
	sprite_ = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	sprite_->Initialize(spriteCommon_, "Resources/uvChecker.png");
	sprite_->SetAnchorPoint({ 0.5f,0.5f });
	sprite_->SetFlipX(true);

	sprite2_ = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	sprite2_->Initialize(spriteCommon_, "Resources/monsterBall.png");
	sprite2Position = { 100.0f,100.0f };
	sprite2_->SetPosition(sprite2Position);
	sprite2_->SetSize({ 300.0f,300.0f });

	ModelManager::GetInstance()->LoadModel("plane.obj");
	object3d_ = new Object3d();
	object3d_->SetModel("plane.obj");
	object3d_->Initialize(object3dCommon_);

	ModelManager::GetInstance()->LoadModel("axis.obj");
	object3d2_ = new Object3d();
	object3d2_->SetModel("axis.obj");
	object3d2_->Initialize(object3dCommon_);

	audio_ = new Audio();
	audio_->Initialize(audioCommon_, "Alarm01.wav");


}

void MyGame::Finalize()
{

	delete audio_;
	delete object3d_;
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
	object3d_->Update();
	object3d2_->Update();

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
	object3dCommon_->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	object3d_->Draw();
	object3d2_->Draw();

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
	///------------------------------///

	//スプライトの共通描画設定
	spriteCommon_->SettingCommonDrawing();

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

