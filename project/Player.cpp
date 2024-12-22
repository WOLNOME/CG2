#include "Player.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::Initialize()
{
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	//モデルの生成と初期化
	model_ = std::make_unique<Object3d>();
	model_->InitializeModel("snowplow", OBJ);


}

void Player::Update()
{
	worldTransform_.translate_.z += 0.01f;

	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();
}

void Player::Draw(const BaseCamera& camera, const SceneLight* light)
{
	//モデルの描画
	model_->Draw(worldTransform_, camera, light);

}
