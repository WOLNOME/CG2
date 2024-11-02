#include "Framework.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Input.h"
#include "AudioCommon.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "SceneManager.h"

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
	Input::GetInstance()->Initialize(winApp_);

	//オーディオ共通部
	AudioCommon::GetInstance()->Initialize();

	//スプライト共通部
	SpriteCommon::GetInstance()->Initialize(dxCommon_);

	//オブジェクト3D共通部
	Object3dCommon::GetInstance()->Initialize(dxCommon_);

	//カメラの生成
	camera_ = new Camera();
	camera_->SetRotate({ 0.0f,0.0f,0.0f });
	camera_->SetTranslate({ 0.0f,0.0f,-15.0f });
	Object3dCommon::GetInstance()->SetDefaultCamera(camera_);

	//シーンマネージャーの生成
	SceneManager::GetInstance()->Initialize();

}

void Framework::Finalize()
{
	SceneManager::GetInstance()->Finalize();
	delete camera_;
	Object3dCommon::GetInstance()->Finalize();
	SpriteCommon::GetInstance()->Finalize();
	AudioCommon::GetInstance()->Finalize();
	Input::GetInstance()->Finalize();

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
	Input::GetInstance()->Update();
	//カメラの更新
	camera_->Update();
	//シーンマネージャー更新
	SceneManager::GetInstance()->Update();

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
