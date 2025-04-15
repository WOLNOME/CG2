#include "Framework.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "D2DRender.h"
#include "SrvManager.h"
#include "TextWriteManager.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "Input.h"
#include "AudioCommon.h"
#include "SpriteCommon.h"
#include "Object3dCommon.h"
#include "LineDrawerCommon.h"
#include "SceneManager.h"

void Framework::Initialize()
{
	//解放処理確認用
	leakChecker;

	//WindowsAPIの初期化
	WinApp::GetInstance()->Initialize();

	//DirectX12
	DirectXCommon::GetInstance()->Initialize();

	//メインレンダー
	MainRender::GetInstance()->Initialize();

	//D2Dレンダー
	D2DRender::GetInstance()->Initialize();

	//SRVマネージャー
	SrvManager::GetInstance()->Initialize();

	//テキストライトマネージャー
	TextWriteManager::GetInstance()->Initialize();

	//ImGuiマネージャー
	ImGuiManager::GetInstance()->Initialize();

	//テクスチャマネージャー
	TextureManager::GetInstance()->Initialize();

	//モデルマネージャー
	ModelManager::GetInstance()->Initialize();

	//パーティクルマネージャー
	ParticleManager::GetInstance()->Initialize();

	//インプット
	Input::GetInstance()->Initialize();

	//オーディオ共通部
	AudioCommon::GetInstance()->Initialize();

	//スプライト共通部
	SpriteCommon::GetInstance()->Initialize();

	//オブジェクト3D共通部
	Object3dCommon::GetInstance()->Initialize();

	//線描画共通部
	LineDrawerCommon::GetInstance()->Initialize();

	//シーンマネージャーの生成
	SceneManager::GetInstance()->Initialize();

}

void Framework::Finalize()
{
	SceneManager::GetInstance()->Finalize();
	LineDrawerCommon::GetInstance()->Finalize();
	Object3dCommon::GetInstance()->Finalize();
	SpriteCommon::GetInstance()->Finalize();
	AudioCommon::GetInstance()->Finalize();
	Input::GetInstance()->Finalize();
	ParticleManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	ImGuiManager::GetInstance()->Finalize();
	TextWriteManager::GetInstance()->Finalize();
	SrvManager::GetInstance()->Finalize();
	D2DRender::GetInstance()->Finalize();
	MainRender::GetInstance()->Finalize();
	DirectXCommon::GetInstance()->Finalize();
	WinApp::GetInstance()->Finalize();
}

void Framework::Update()
{
	//メッセージ処理
	if (WinApp::GetInstance()->ProcessMessage()) {
		isOver = true;
	}
	//インプット更新
	Input::GetInstance()->Update();
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
