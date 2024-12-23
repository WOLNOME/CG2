#include "Player.h"
#undef min
#undef max
#include <algorithm>

Player::Player()
{
}

Player::~Player()
{
}

void Player::Initialize()
{
	//インプット
	input_ = Input::GetInstance();
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	//モデルの生成と初期化
	model_ = std::make_unique<Object3d>();
	model_->InitializeModel("snowplow", OBJ);


}

void Player::Update()
{
	//移動に関する処理
	Move();


	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();
}

void Player::Draw(const BaseCamera& camera, const SceneLight* light)
{
	//モデルの描画
	model_->Draw(worldTransform_, camera, light);

}

void Player::Move()
{
	bool isMove = false;
	//移動
	Vector3 velocity = { 0.0f,0.0f,0.0f };
	if (input_->PushKey(DIK_W)) {
		velocity.z += speed_;
		isMove = true;
	}
	if (input_->PushKey(DIK_A)) {
		velocity.x += -speed_;
		isMove = true;
	}
	if (input_->PushKey(DIK_S)) {
		velocity.z += -speed_;
		isMove = true;
	}
	if (input_->PushKey(DIK_D)) {
		velocity.x += speed_;
		isMove = true;
	}
	//速度の合成
	worldTransform_.translate_ += velocity;

	if (isMove && (velocity.x != 0.0f || velocity.y != 0.0f || velocity.z != 0.0f)) {
		//回転
		Vector3 direction = { 0.0f,0.0f,1.0f };
		direction = MyMath::TransformNormal(direction, worldTransform_.matWorld_);
		direction.y = 0.0f;
		direction.Normalize();
		velocity.Normalize();

		//directionとvelocityのなす角thetaを求める
		float theta = MyMath::AngleOf2VectorY(direction, velocity);

		if (theta < 0.0f) {
			worldTransform_.rotate_.y -= rotateSpeed_;
		}
		else {
			worldTransform_.rotate_.y += rotateSpeed_;
		}

		//thetaが一定の大きさならvelocityの方向を向く
		if (theta<rotateSpeed_ && theta>-rotateSpeed_) {
			worldTransform_.rotate_.y = MyMath::AngleOf2VectorY(Vector3(0.0f, 0.0f, 1.0f), velocity);
		}
	}


}

