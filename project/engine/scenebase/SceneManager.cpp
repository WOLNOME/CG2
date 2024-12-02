#include "SceneManager.h"
#include "SceneFactory.h"
#include <cassert>

SceneManager* SceneManager::instance = nullptr;

SceneManager* SceneManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new SceneManager;
	}
	return instance;
}

void SceneManager::Initialize()
{
	//シーンファクトリーの生成
	sceneFactory_ = new SceneFactory();
}

void SceneManager::Update()
{
	//シーン切り替え処理
	ChangeScene();
	//実行中シーンを更新する
	scene_->Update();
}

void SceneManager::ShadowMapDraw()
{
	//シャドウマップのレンダリング
	scene_->ShadowMapDraw();
}

void SceneManager::Draw()
{
	//シーンの描画
	scene_->Draw();
}

void SceneManager::Finalize()
{
	//最後のシーンの終了と解放
	scene_->Finalize();
	delete scene_;
	//シーンファクトリー解放
	delete sceneFactory_;
	//インスタンスの削除
	delete instance;
	instance = nullptr;
}

void SceneManager::ChangeScene()
{
	//次のシーン予約があるなら
	if (nextScene_) {
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
}

void SceneManager::SetNextScene(const std::string& nextSceneName)
{
	//警告
	assert(sceneFactory_);
	assert(nextScene_ == nullptr);
	//次シーンを生成
	nextScene_ = sceneFactory_->CreateScene(nextSceneName);
}
