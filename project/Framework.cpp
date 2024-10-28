#include "Framework.h"
#include "TextureManager.h"
#include "ModelManager.h"

void Framework::Initialize()
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

void Framework::Finalize()
{
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

void Framework::Update()
{
	//メッセージ処理
	if (winApp_->ProcessMessage()) {
		isOver = true;
	}
	input_->Update();
	//カメラの更新
	camera_->Update();

}

void Framework::Run()
{
	//ゲームの初期化
	Initialize();
	while (true)
	{
		//毎フレーム更新
		Update();
		//終了リクエストが来たら抜ける
		if (GetOver()) {
			break;
		}
		//描画
		Draw();
	}
	//ゲームの終了
	Finalize();
}
