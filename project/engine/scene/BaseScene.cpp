#include "BaseScene.h"
#include "SceneManager.h"

void BaseScene::Initialize()
{
	//シーンの初期化
	sceneManager_ = SceneManager::GetInstance();
	//シーンライトの生成と初期化
	sceneLight_ = std::make_unique<SceneLight>();
	sceneLight_->Initialize();

}

void BaseScene::Finalize()
{
}

void BaseScene::Update()
{
	//シーンライトの更新
	sceneLight_->Update();
}

void BaseScene::Draw()
{
}
