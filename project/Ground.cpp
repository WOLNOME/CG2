#include "Ground.h"

Ground::Ground()
{
}

Ground::~Ground()
{
}

void Ground::Initialize()
{
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	//モデルの生成と初期化
	model_ = std::make_unique<Object3d>();
	model_->Initialize("ground", OBJ);
}

void Ground::Update()
{
	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();
}

void Ground::Draw(const BaseCamera& camera, const SceneLight* light)
{
	//モデル
	model_->Draw(worldTransform_, camera, light);
}
