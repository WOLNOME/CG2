#include "MyGame.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Model.h"

void MyGame::Initialize()
{
	//ゲーム基盤部の初期化
	Framework::Initialize();

	//シーンの生成初期化
	gamePlayScene_ = std::make_unique<GamePlayScene>();
	gamePlayScene_->Initialize();

}

void MyGame::Finalize()
{
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

	//シーンの更新
	gamePlayScene_->Update();


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

	//シーンの描画
	gamePlayScene_->Draw();

	//ImGuiの描画
	imGuiManager_->Draw();

	//描画後処理
	dxCommon_->PostDraw();
}

