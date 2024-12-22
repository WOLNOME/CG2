#include "Skydome.h"

Skydome::Skydome()
{
}

Skydome::~Skydome()
{
}

void Skydome::Initialize()
{
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	//モデルの生成と初期化
	model_ = std::make_unique<Object3d>();
	model_->InitializeModel("skydome", OBJ);

}

void Skydome::Update()
{
	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();
}

void Skydome::Draw(const BaseCamera& camera, const SceneLight* light)
{
	//モデル
	model_->Draw(worldTransform_, camera, light);
}
