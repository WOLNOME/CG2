#include "SceneManager.h"

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
}

void SceneManager::Update()
{
	//シーン切り替え
	ChangeScene();
	//実行中シーンを更新する
	scene_->Update();

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
