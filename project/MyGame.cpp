#include "MyGame.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "TextWriteManager.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "Model.h"
#include "SceneManager.h"

void MyGame::Initialize()
{
	//ゲーム基盤部の初期化
	Framework::Initialize();

	//シーンマネージャーに最初のシーンをセット
	SceneManager::GetInstance()->SetNextScene("PARTICLECREATOR");

	//パーティクルエディター→PARTICLECREATOR
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

	//パーティクルマネージャーの更新
	ParticleManager::GetInstance()->Update();

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

	//シーンのパーティクル描画
	ParticleManager::GetInstance()->Draw();

	//ImGuiの描画
	ImGuiManager::GetInstance()->Draw();

	//描画後処理
	MainRender::GetInstance()->PostDraw();

	///------------------------------///
	///        テキスト描画処理
	///------------------------------///

	//テキスト描画前処理
	TextWriteManager::GetInstance()->BeginDrawWithD2D();
	//シーンの文字描画
	SceneManager::GetInstance()->TextDraw();
	//テキスト描画後処理
	TextWriteManager::GetInstance()->EndDrawWithD2D();

	///------------------------------///
	///        全ての描画が終了
	///------------------------------///

	//画面切り替え
	MainRender::GetInstance()->ExchangeScreen();
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

