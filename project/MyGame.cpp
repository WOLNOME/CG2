#include "MyGame.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "Model.h"
#include "SceneManager.h"

void MyGame::Initialize()
{
	//ゲーム基盤部の初期化
	Framework::Initialize();

	//シーンマネージャーに最初のシーンをセット
	SceneManager::GetInstance()->SetNextScene("TITLE");

}

void MyGame::Finalize()
{
	//ゲーム基盤解放
	Framework::Finalize();
}

void MyGame::Update()
{
	//ImGui受付開始
	ImGuiManager::GetInstance()->Begin();

	//ゲーム基盤更新(シーンの処理もここ、ImGuiの処理も更新処理で)
	Framework::Update();

	//ImGuiの内部コマンドを生成する
	ImGuiManager::GetInstance()->End();
}

void MyGame::Draw()
{
	///==============================///
	///          描画処理
	///==============================///

	
	///------------------------------///
	///        メインレンダー
	///------------------------------///

	//描画前処理
	MainRender::GetInstance()->PreDraw();
	SrvManager::GetInstance()->PreDraw(MainRender::GetInstance()->GetCommandList());

	//シーンの描画
	SceneManager::GetInstance()->Draw();

	//ImGuiの描画
	ImGuiManager::GetInstance()->Draw();

	//描画後処理
	MainRender::GetInstance()->PostDraw();
	//単レンダー終了時の共通処理
	DirectXCommon::GetInstance()->PostEachRender();
	//コマンドのリセット
	MainRender::GetInstance()->ReadyNextCommand();

	///------------------------------///
	///      レンダーの最終処理
	///------------------------------///

	//全レンダー終了時の共通処理
	DirectXCommon::GetInstance()->PostAllRenders();

}

