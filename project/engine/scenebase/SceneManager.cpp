#include "SceneManager.h"
#include "SceneFactory.h"
#include "ImGuiManager.h"
#include <cassert>

SceneManager* SceneManager::instance = nullptr;

SceneManager* SceneManager::GetInstance() {
	if (instance == nullptr) {
		instance = new SceneManager;
	}
	return instance;
}

void SceneManager::Initialize() {
	//シーンファクトリーの生成
	sceneFactory_ = new SceneFactory();
	//シーン遷移アニメーションの生成
	sceneTransitionAnimation_ = std::make_unique<SceneTransitionAnimation>();
	sceneTransitionAnimation_->Initialize();

}

void SceneManager::Update() {
	//シーン遷移アニメーションの更新
	sceneTransitionAnimation_->Update();
	//シーン切り替え処理
	ChangeScene();
	//実行中シーンを更新する
	scene_->Update();
	//FPS表示
	scene_->ShowFPS();

}

void SceneManager::Draw() {
	//シーンの描画
	scene_->Draw();
	//シーン遷移アニメーションの描画
	sceneTransitionAnimation_->Draw();
}

void SceneManager::TextDraw() {
	//シーンの文字描画
	scene_->TextDraw();
}

void SceneManager::Finalize() {
	//最後のシーンの終了と解放
	scene_->Finalize();
	delete scene_;
	//シーンファクトリー解放
	delete sceneFactory_;
	//インスタンスの削除
	delete instance;
	instance = nullptr;
}

void SceneManager::ChangeScene() {
	//次のシーン予約があるなら
	if (nextScene_ && !sceneTransitionAnimation_->IsTransitioning()) {
		//遷移アニメーション開始
		sceneTransitionAnimation_->StartTransition();
	}
	//遷移アニメーション中なら
	if (sceneTransitionAnimation_->GetState() == SceneTransitionAnimation::TransitionState::UPDATE_IN) {
		//フェードイン処理
		sceneTransitionAnimation_->UpdateIn();
	}
	else if (sceneTransitionAnimation_->GetState() == SceneTransitionAnimation::TransitionState::END_IN) {
		//フェードイン終了
		sceneTransitionAnimation_->EndIn();
		//旧シーンの終了
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}
		//シーンの切り替え
		scene_ = nextScene_;
		nextScene_ = nullptr;
		//次のシーンを初期化する
		scene_->Initialize();
	}
	else if (sceneTransitionAnimation_->GetState() == SceneTransitionAnimation::TransitionState::UPDATE_OUT) {
		//フェードアウト処理
		sceneTransitionAnimation_->UpdateOut();
	}
	else if (sceneTransitionAnimation_->GetState() == SceneTransitionAnimation::TransitionState::END_OUT) {
		//フェードアウト終了
		sceneTransitionAnimation_->EndOut();
	}
	else if (sceneTransitionAnimation_->GetState() == SceneTransitionAnimation::TransitionState::END_ALL) {
		//フェードアウト終了
		sceneTransitionAnimation_->EndAll();
	}
}

void SceneManager::SetNextScene(const std::string& nextSceneName, SceneTransitionAnimation::TransitionType transitionType, uint32_t frame) {
	//遷移中なら何もしない
	if (sceneTransitionAnimation_->IsTransitioning()) return;

	//警告
	assert(sceneFactory_);
	assert(nextScene_ == nullptr);

	//もし最初のシーンだったらここで生成＆初期化
	if (scene_ == nullptr) {
		scene_ = sceneFactory_->CreateScene(nextSceneName);
		nextScene_ = nullptr;
		scene_->Initialize();
		return;
	}

	//次シーンを生成
	nextScene_ = sceneFactory_->CreateScene(nextSceneName);
	//遷移アニメーションタイプを設定
	sceneTransitionAnimation_->SetTransitionType(transitionType);
	//遷移アニメーションフレームを設定
	sceneTransitionAnimation_->SetFrame(frame);
}
