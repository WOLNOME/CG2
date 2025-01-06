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
	worldTransform_.translate_.y = 2.5f;
	//モデルの生成と初期化
	model_ = std::make_unique<Object3d>();
	model_->InitializeModel("snowplow", OBJ);

	//弾の生成と初期化
	snowBullet_ = std::make_unique<SnowBullet>();
	snowBullet_->Initialize();
	snowBullet_->SetParent(&worldTransform_);

}

void Player::Update()
{
	//移動に関する処理
	Move();
	//発射に関する処理
	Fire();
	//弾に関する処理
	Bullet();

	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();
}

void Player::Draw(const BaseCamera& camera, const SceneLight* light)
{
	//モデルの描画
	model_->Draw(worldTransform_, camera, light);
	//弾の描画
	if (snowBullet_) {
		snowBullet_->Draw(camera, light);
	}

}

void Player::DrawLine(const BaseCamera& camera)
{
	if (snowBullet_) {
		snowBullet_->DrawLine(camera);
	}
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
	//移動制限
	worldTransform_.translate_.x = std::max(worldTransform_.translate_.x, -30.0f);
	worldTransform_.translate_.x = std::min(worldTransform_.translate_.x, 30.0f);
	worldTransform_.translate_.z = std::max(worldTransform_.translate_.z, -20.0f);
	worldTransform_.translate_.z = std::min(worldTransform_.translate_.z, 20.0f);

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

	//弾に関する処理
	if (snowBullet_) {
		//射出されてなかったら
		if (!snowBullet_->isFire_) {
			//プレイヤーが移動してたら
			if (isMove) {
				//弾の移動フラグをオン
				snowBullet_->isMove_ = true;
			}
			else {
				//弾の移動フラグをオフ
				snowBullet_->isMove_ = false;
			}
		}
	}

}

void Player::Fire()
{
	//未発射のとき
	if (!isFire_) {
		if (input_->TriggerKey(DIK_SPACE)) {
			isFire_ = true;
			//弾の発射フラグをオンにする
			snowBullet_->isFire_ = true;
			//発射向きを送る
			Vector3 direction;
			direction = MyMath::TransformNormal({ 0.0f,0.0f,1.0f }, worldTransform_.matWorld_);
			direction.y = 0.0f;
			snowBullet_->SetDirection(direction);
			//弾とのペアレントを解除
			snowBullet_->CancelParent();
		}
	}
}

void Player::Bullet()
{
	//弾が生きてたら
	if (snowBullet_) {
		//死亡処理
		if (snowBullet_->GetIsDead()) {
			snowBullet_.reset();
			//未発射状態にする
			isFire_ = false;
			//直ちに新しい弾を生成&初期化する
			snowBullet_ = std::make_unique<SnowBullet>();
			snowBullet_->Initialize();
			snowBullet_->SetParent(&worldTransform_);
		}
		//更新
		snowBullet_->Update();
	}
}

