#include "BaseScene.h"
#include "SceneManager.h"

void BaseScene::Initialize()
{
	//シーンの初期化
	sceneManager_ = SceneManager::GetInstance();
}

void BaseScene::Finalize()
{
}

void BaseScene::Update()
{
}

void BaseScene::Draw()
{
}
