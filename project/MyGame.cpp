#include "MyGame.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Model.h"

void MyGame::Initialize()
{
	//ゲーム基盤部の初期化
	BaseInitialize();

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
	delete camera_;
	delete object3dCommon_;
	delete spriteCommon_;
	delete audioCommon_;
	delete input_;
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	imGuiManager_->Finalize();
	delete imGuiManager_;
	delete srvManager_;
	delete dxCommon_;
	//WindowsAPIの終了処理
	winApp_->Finalize();
	delete winApp_;

}

void MyGame::Update()
{
	//メッセージ処理
	if (winApp_->ProcessMessage()) {
		isOver = true;
	}

	//ImGui受付開始
	imGuiManager_->Begin();

	input_->Update();


	//ゲームの処理

	///==============================///
	///          更新処理
	///==============================///

	//カメラの更新
	camera_->Update();

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
	///          モデル描画開始
	///------------------------------///

	object3d_->Draw();
	object3d2_->Draw();

	///------------------------------///
	///          モデル描画終了
	///------------------------------///

	//スプライトの共通描画設定
	spriteCommon_->SettingCommonDrawing();

	///------------------------------///
	///          スプライト描画開始
	///------------------------------///

	//スプライト描画
	sprite_->Draw();
	sprite2_->Draw();

	//ImGuiの描画
	imGuiManager_->Draw();

	///------------------------------///
	///          スプライト描画終了
	///------------------------------///

	//描画後処理
	dxCommon_->PostDraw();
}

void MyGame::BaseInitialize()
{
	//解放処理確認用
	leakChecker;

	//ウィンドウ
	//WindowsAPIの初期化
	winApp_ = new WinApp();
	winApp_->Initialize();

	//DorectX12
	dxCommon_ = new DirectXCommon();
	dxCommon_->Initialize(winApp_);

	//SRVマネージャー
	srvManager_ = new SrvManager();
	srvManager_->Initialize(dxCommon_);

	//ImGuiマネージャー
	imGuiManager_ = new ImGuiManager();
	imGuiManager_->Initialize(dxCommon_, winApp_, srvManager_);

	//テクスチャマネージャ
	TextureManager::GetInstance()->Initialize(dxCommon_, srvManager_);

	//モデルマネージャー
	ModelManager::GetInstance()->Initialize(dxCommon_);

	//インプット
	input_ = new Input();
	input_->Initialize(winApp_);

	//オーディオ共通部
	audioCommon_ = new AudioCommon();
	audioCommon_->Initialize();

	//スプライト共通部
	spriteCommon_ = new SpriteCommon();
	spriteCommon_->Initialize(dxCommon_);

	//オブジェクト3D共通部
	object3dCommon_ = new Object3dCommon();
	object3dCommon_->Initialize(dxCommon_);

	//カメラの生成
	camera_ = new Camera();
	camera_->SetRotate({ 0.0f,0.0f,0.0f });
	camera_->SetTranslate({ 0.0f,0.0f,-15.0f });
	object3dCommon_->SetDefaultCamera(camera_);

}
