#include "MyGame.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "D2DRender.h"
#include "PostEffectManager.h"
#include "TextureManager.h"
#include "GPUDescriptorManager.h"
#include "TextTextureManager.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "SceneManager.h"

void MyGame::Initialize() {
	//ゲーム基盤部の初期化
	Framework::Initialize();

	//シーンマネージャーに最初のシーンをセット
	SceneManager::GetInstance()->SetNextScene("DEVELOP");

	//パーティクルエディター→PARTICLECREATOR
	//開発用シーン→DEVELOP
}

void MyGame::Finalize() {
	//ゲーム基盤解放
	Framework::Finalize();
}

void MyGame::Update() {
	//ImGui受付開始
	ImGuiManager::GetInstance()->Begin();

	//ゲーム基盤更新(シーンの処理もここ、ImGuiの処理も更新処理で)
	Framework::Update();

	//パーティクルマネージャーの更新
	ParticleManager::GetInstance()->Update();

	//ImGuiの内部コマンドを生成する
	ImGuiManager::GetInstance()->End();
}

void MyGame::Draw() {
	///==============================///
	///          描画処理
	///==============================///

	///------------------------------///
	///   テキストテクスチャの生成処理
	///------------------------------///

	//テキストテクスチャ描画前処理
	TextTextureRender::GetInstance()->PreDraw();
	GPUDescriptorManager::GetInstance()->PreDraw(TextTextureRender::GetInstance()->GetCommandList());

	//文字をD2D描画でテクスチャに書き込む
	TextTextureManager::GetInstance()->WriteTextOnD2D();
	//文字の装飾をD3D12で行う
	TextTextureManager::GetInstance()->DrawDecorationOnD3D12();

	//テキストテクスチャ描画後処理
	TextTextureRender::GetInstance()->PostDraw();
	DirectXCommon::GetInstance()->PostEachRender();			//GPUの実行を待つ
	TextTextureRender::GetInstance()->ReadyNextCommand();	//TextTextureRenderで使用したコマンドをリセット


	///------------------------------///
	///        D3D12の描画処理
	///------------------------------///

	//オブジェクト描画前処理
	PostEffectManager::GetInstance()->PreObjectDraw();
	MainRender::GetInstance()->PreObjectDraw();
	GPUDescriptorManager::GetInstance()->PreDraw(MainRender::GetInstance()->GetCommandList());

	//シーンの描画
	SceneManager::GetInstance()->Draw();

	//シーンのパーティクル描画
	ParticleManager::GetInstance()->Draw();

	//ImGui描画前処理
	MainRender::GetInstance()->PreImGuiDraw();
	PostEffectManager::GetInstance()->CopySceneToRenderTexture();

	//ImGuiの描画
	ImGuiManager::GetInstance()->Draw();

	//描画後処理
	TextTextureManager::GetInstance()->ReadyNextResourceState();		//MainRenderの描画が終了した時点でtextTextureResourceのステートを遷移
	MainRender::GetInstance()->PostDraw();		//GPUにMainRenderの描画処理を投げる
	DirectXCommon::GetInstance()->PostEachRender();		//GPUの実行を待つ
	MainRender::GetInstance()->ReadyNextCommand();		//MainRenderで使用したコマンドをリセット

	///------------------------------///
	///        D2Dの描画処理
	///------------------------------///

	//D2Dの描画前処理
	D2DRender::GetInstance()->PreDraw();

	//シーン遷移アニメーションの描画(一番上に描画)
	SceneManager::GetInstance()->CurtainDraw();

	//D2Dの描画後処理
	D2DRender::GetInstance()->PostDraw();

	///------------------------------///
	///        全ての描画が終了
	///------------------------------///

	//画面切り替え
	MainRender::GetInstance()->ExchangeScreen();
	
	///------------------------------///
	///      レンダーの最終処理
	///------------------------------///

	//全レンダー終了時の共通処理
	DirectXCommon::GetInstance()->PostAllRenders();

}

