#include "BaseScene.h"
#include "SceneManager.h"
#include <ImGuiManager.h>

BaseScene::~BaseScene() {
}

void BaseScene::Initialize() {
	//シーンの初期化
	sceneManager_ = SceneManager::GetInstance();
	//シーンライトの生成と初期化
	sceneLight_ = std::make_unique<SceneLight>();
	sceneLight_->Initialize();

}

void BaseScene::Finalize() {
}

void BaseScene::Update() {
	//シーンの更新
	sceneLight_->Update();
}

void BaseScene::Draw() {
}

void BaseScene::ShowFPS() {
	//現在時間の取得
	auto now = std::chrono::steady_clock::now();
	// 経過時間を計算
	std::chrono::duration<float> deltaTime = now - lastFrameTime_;
	lastFrameTime_ = now;

	// FPSを計算
	fps_ = 1.0f / deltaTime.count();

	// ImGuiで表示
#ifdef _DEBUG
	ImGui::Begin("FPS Display");
	ImGui::Text("Current FPS: %.0f", fps_);
	ImGui::End();
#endif // _DEBUG

}
