#include "Framework.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Input.h"
#include "AudioCommon.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "SceneManager.h"

void Framework::Initialize()
{
	//解放処理確認用
	leakChecker;

	//WindowsAPIの初期化
	WinApp::GetInstance()->Initialize();

	//DorectX12
	DirectXCommon::GetInstance()->Initialize();

	//SRVマネージャー
	SrvManager::GetInstance()->Initialize();

	//ImGuiマネージャー
	ImGuiManager::GetInstance()->Initialize();

	//テクスチャマネージャ
	TextureManager::GetInstance()->Initialize();

	//モデルマネージャー
	ModelManager::GetInstance()->Initialize();

	//インプット
	Input::GetInstance()->Initialize();

	//オーディオ共通部
	AudioCommon::GetInstance()->Initialize();

	//スプライト共通部
	SpriteCommon::GetInstance()->Initialize();

	//オブジェクト3D共通部
	Object3dCommon::GetInstance()->Initialize();

	//パーティクル共通部
	ParticleCommon::GetInstance()->Initialize();

	//カメラの生成
	camera_ = std::make_unique<Camera>();
	camera_->SetRotate({ 0.0f,0.0f,0.0f });
	camera_->SetTranslate({ 0.0f,0.0f,-15.0f });
	Object3dCommon::GetInstance()->SetDefaultCamera(camera_.get());
	ParticleCommon::GetInstance()->SetDefaultCamera(camera_.get());

	//シーンマネージャーの生成
	SceneManager::GetInstance()->Initialize();

}

void Framework::Finalize()
{
	SceneManager::GetInstance()->Finalize();
	camera_.reset();
	Object3dCommon::GetInstance()->Finalize();
	SpriteCommon::GetInstance()->Finalize();
	AudioCommon::GetInstance()->Finalize();
	Input::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	ImGuiManager::GetInstance()->Finalize();
	SrvManager::GetInstance()->Finalize();
	DirectXCommon::GetInstance()->Finalize();
	WinApp::GetInstance()->Finalize();
}

void Framework::Update()
{
	//メッセージ処理
	if (WinApp::GetInstance()->ProcessMessage()) {
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
