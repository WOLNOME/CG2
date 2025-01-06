#include "SnowBullet.h"
#include "TextureManager.h"
#undef min
#undef max
#include <algorithm>
#include "CollisionConfig.h"

void SnowBullet::Initialize()
{
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	//モデル系の生成と初期化
	textureHandle_ = TextureManager::GetInstance()->LoadTexture("snowBall.png");
	model_ = std::make_unique<Object3d>();
	model_->InitializeShape(Shape::kSphere);
	//メンバ変数の初期化
	speed_ = 0.5f;
	isFire_ = false;
	isMove_ = false;
	isDead_ = false;
	//コライダー関連の初期化
	SetCollisionAttribute(kCollisionAttributePlayerBullet);
	radius_ = 0.5f;
	debugLine_ = std::make_unique<LineDrawer>();
	debugLine_->Initialize();
}

void SnowBullet::Update()
{
	//射出前処理
	if (!isFire_) {
		//プレイヤーが移動している場合
		if (isMove_) {
			radius_ += 0.003f;
			//雪玉の最大値で留まらせる
			radius_ = std::min(radius_, 5.0f);

			//回転
			worldTransform_.rotate_.x += 0.03f;

		}

		//半径に応じて座標を変換
		worldTransform_.translate_.x = 0.0f;
		worldTransform_.translate_.y = -2.5f + radius_;
		worldTransform_.translate_.z = 4.3f + radius_;

	}
	//射出中処理
	else {
		//ローカル座標をワールド座標から参照する
		worldTransform_.translate_ = worldTransform_.worldTranslate_;
		worldTransform_.translate_.y = radius_;
		//半径を大きくする
		radius_ += 0.009f;
		//雪玉の最大値で留まらせる
		radius_ = std::min(radius_, 5.0f);
		//回転
		worldTransform_.rotate_.x += 0.05f;
		//移動
		worldTransform_.translate_ += direction_.Normalized() * speed_;

		//移動制限
		if (worldTransform_.translate_.x > 60.0f ||
			worldTransform_.translate_.x < -60.0f ||
			worldTransform_.translate_.z > 120.0f ||
			worldTransform_.translate_.z < -60.0f
			) {
			isDead_ = true;
		}

	}

	//半径をワールドトランスフォームのスケールに反映
	worldTransform_.scale_ = { radius_,radius_,radius_ };
	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();

#ifdef _DEBUG
	debugColor_ = { 0.0f,0.0f,1.0f,1.0f };

#endif // _DEBUG

}

void SnowBullet::Draw(const BaseCamera& camera, const SceneLight* light)
{
	//描画
	model_->Draw(worldTransform_, camera, light, textureHandle_);
}

void SnowBullet::DrawLine(const BaseCamera& camera)
{
#ifdef _DEBUG
	Sphere sphere;
	sphere.center = worldTransform_.worldTranslate_;
	sphere.radius = radius_;
	MyMath::DrawSphere(sphere, debugColor_, debugLine_.get(), 10);
	debugLine_->Draw(camera);
#endif // _DEBUG
}

void SnowBullet::OnCollision()
{
	isDead_ = true;
	debugColor_ = { 1.0f,0.0f,0.0f,1.0f };
}

Vector3 SnowBullet::GetWorldPosition()
{
	return worldTransform_.worldTranslate_;
}
