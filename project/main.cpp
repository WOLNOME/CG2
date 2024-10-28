#include <fstream>
#include <xaudio2.h>
#include "DirectXCommon.h"
#include "WinApp.h"
#include "Input.h"
#include "D3DResourceLeakChecker.h"
#include "Logger.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"
#include "Function.h"
#include "Camera.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "AudioCommon.h"
#include "Audio.h"

#pragma comment(lib,"xaudio2.lib")

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#pragma region 基盤システムの初期化
	//解放処理確認用
	D3DResourceLeakChecker leakChecker;

	//ウィンドウ
	WinApp* winApp = nullptr;
	//WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	//DorectX12
	DirectXCommon* dxCommon = nullptr;
	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//SRVマネージャー
	SrvManager* srvManager = nullptr;
	srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);

	//ImGuiマネージャー
	ImGuiManager* imGuiManager = nullptr;
	imGuiManager = new ImGuiManager();
	imGuiManager->Initialize(dxCommon, winApp, srvManager);

	//テクスチャマネージャ
	TextureManager::GetInstance()->Initialize(dxCommon, srvManager);

	//モデルマネージャー
	ModelManager::GetInstance()->Initialize(dxCommon);

	//インプット
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp);

	//オーディオ共通部
	AudioCommon* audioCommon = nullptr;
	audioCommon = new AudioCommon();
	audioCommon->Initialize();

	//スプライト共通部
	SpriteCommon* spriteCommon = nullptr;
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	//オブジェクト3D共通部
	Object3dCommon* object3dCommon = nullptr;
	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);

	//カメラの生成
	Camera* camera = new Camera();
	camera->SetRotate({ 0.0f,0.0f,0.0f });
	camera->SetTranslate({ 0.0f,0.0f,-15.0f });
	object3dCommon->SetDefaultCamera(camera);



#pragma endregion 基盤システムの初期化

	//Transform変数を作る
	Transform transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};


	//カメラの位置、角度を作る
	Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-10.0f}
	};

	//初期化
#pragma region 最初のシーン初期化
	Sprite* sprite = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	sprite->Initialize(spriteCommon, "Resources/uvChecker.png");
	sprite->SetAnchorPoint({ 0.5f,0.5f });
	sprite->SetFlipX(true);

	Sprite* sprite2 = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	sprite2->Initialize(spriteCommon, "Resources/monsterBall.png");
	Vector2 sprite2Position = { 100.0f,100.0f };
	sprite2->SetPosition(sprite2Position);
	sprite2->SetSize({ 300.0f,300.0f });

	ModelManager::GetInstance()->LoadModel("plane.obj");
	Object3d* object3d = new Object3d();
	object3d->SetModel("plane.obj");
	object3d->Initialize(object3dCommon);

	ModelManager::GetInstance()->LoadModel("axis.obj");
	Object3d* object3d2 = new Object3d();
	object3d2->SetModel("axis.obj");
	object3d2->Initialize(object3dCommon);

	Audio* audio = new Audio();
	audio->Initialize(audioCommon, "Alarm01.wav");
	
#pragma endregion 最初のシーンの終了

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		//メッセージ処理
		if (winApp->ProcessMessage()) {
			break;
		}

		//ImGui受付開始
		imGuiManager->Begin();

		input->Update();


		//ゲームの処理

		///==============================///
		///          更新処理
		///==============================///

		//カメラの更新
		camera->Update();

		//モデルの更新
		object3d->Update();
		object3d2->Update();

		//スプライトの更新
		sprite->Update();
		sprite->SetRotation(sprite->GetRotation() + 0.03f);
		sprite2->Update();

#ifdef _DEBUG
		
		ImGui::SetNextWindowSize(ImVec2(500, 100));
		ImGui::Begin("MosterBall");
		ImGui::SliderFloat2("position", &sprite2Position.x,0.0f,1200.0f,"%5.1f");
		sprite2->SetPosition(sprite2Position);
		ImGui::End();

		ImGui::Begin("Audio");
		if (ImGui::Button("PlayAudio")) {
			audio->Play();
		}
		ImGui::End();

#endif // _DEBUG

		//ImGuiの内部コマンドを生成する
		imGuiManager->End();

		///==============================///
		///          描画処理
		///==============================///

		//描画前処理
		dxCommon->PreDraw();
		srvManager->PreDraw();

		//3Dモデルの共通描画設定
		object3dCommon->SettingCommonDrawing();

		///------------------------------///
		///          モデル描画開始
		///------------------------------///

		object3d->Draw();
		object3d2->Draw();

		///------------------------------///
		///          モデル描画終了
		///------------------------------///

		//スプライトの共通描画設定
		spriteCommon->SettingCommonDrawing();

		///------------------------------///
		///          スプライト描画開始
		///------------------------------///

		//スプライト描画
		sprite->Draw();
		sprite2->Draw();

		//ImGuiの描画
		imGuiManager->Draw();

		///------------------------------///
		///          スプライト描画終了
		///------------------------------///

		//描画後処理
		dxCommon->PostDraw();
	}

	/////解放処理/////

	delete audio;
	delete object3d;
	delete sprite2;
	delete sprite;
	delete camera;
	delete object3dCommon;
	delete spriteCommon;
	delete audioCommon;
	delete input;
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	imGuiManager->Finalize();
	delete imGuiManager;
	delete srvManager;
	delete dxCommon;
	//WindowsAPIの終了処理
	winApp->Finalize();
	delete winApp;

	return 0;
}
