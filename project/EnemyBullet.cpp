#include "EnemyBullet.h"
#include "TextureManager.h"
#undef min
#undef max
#include <algorithm>
#include <random>
#include "CollisionConfig.h"


EnemyBullet::EnemyBullet()
{
	SetCollisionAttribute(kCollisionAttributeNothingness);
}

void EnemyBullet::Initialize(const Vector3& enemyPosition, const Vector3& playerPosition)
{
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translate_ = enemyPosition;
	//モデル系の生成と初期化
	textureHandle_ = TextureManager::GetInstance()->LoadTexture("snowBall.png");
	model_ = std::make_unique<Object3d>();
	model_->InitializeShape(Shape::kSphere);
	//メンバ変数の初期化
	speed_ = 2.5f;
	isDead_ = false;
	direction_ = playerPosition - enemyPosition;
	debugColor_ = { 0.0f,0.0f,1.0f,1.0f };
	particleTimer_ = 0;
	isParticleActive = false;
	//コライダー関連の初期化
	SetCollisionAttribute(kCollisionAttributeEnemyBullet);
	radius_ = 1.0f;
	debugLine_ = std::make_unique<LineDrawer>();
	debugLine_->Initialize();
	//パーティクル周り
	for (int i = 0; i < kNumParticle_; i++) {
		//パーティクルの生成初期化
		particles_[i] = std::make_unique<Particle>();
		particles_[i]->Initialize("circle");
		//エミッターの初期化
		emitters_[i].count = 3;
		emitters_[i].frequency = 0.3f;
		emitters_[i].frequencyTime = 0.0f;
		emitters_[i].transform = {
			{1.0f,1.0f,1.0f},
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f}
		};
		//フィールドの初期化
		fields_[i].acceleration = { 0.0f,-3.0f,0.0f };
		fields_[i].area.max = { 100.0f,100.0f,100.0f },
			fields_[i].area.min = { -100.0f,-100.0f,-100.0f };
		fields_[i].isActive = true;
	}
}

void EnemyBullet::Update()
{
	//パーティクル処理
	UpdateParticle();

	//半径を大きくする
	radius_ += 0.12f;
	//雪玉の最大値で留まらせる
	radius_ = std::min(radius_, 30.0f);
	//回転
	worldTransform_.rotate_.x += 2.0f;
	//移動
	worldTransform_.translate_ += direction_.Normalized() * speed_;


	//移動制限
	if ((worldTransform_.translate_.x > 60.0f ||
		worldTransform_.translate_.x < -60.0f ||
		worldTransform_.translate_.z > 120.0f ||
		worldTransform_.translate_.z < -60.0f) &&
		!isParticleActive
		) {
		isParticleActive = true;
		for (int i = 0; i < kNumParticle_; i++) {
			//ランダムエンジンの生成
			std::random_device seedGenerator;
			std::mt19937 randomEngine(seedGenerator());
			//エミッター
			emitters_[i].transform.scale = { 1.0f,1.0f,1.0f };
			emitters_[i].transform.rotate = { 0.0f,0.0f,0.0f };
			emitters_[i].transform.translate = worldTransform_.worldTranslate_;
			emitters_[i].count = 2;
			emitters_[i].frequency = 0.003f;
			emitters_[i].frequencyTime = 0.0f;
			//速度
			std::uniform_real_distribution<float> distVelocity(-1.0f, 1.0f);
			particleVelocity_[i] = { distVelocity(randomEngine),distVelocity(randomEngine),distVelocity(randomEngine) };
			particleVelocity_[i] *= 1.5f;
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

void EnemyBullet::Draw(const BaseCamera& camera, const SceneLight* light)
{
	//描画
	if (!isParticleActive) {
		model_->Draw(worldTransform_, camera, light, textureHandle_);
	}
}

void EnemyBullet::DrawLine(const BaseCamera& camera)
{
#ifdef _DEBUG
	Sphere sphere;
	sphere.center = worldTransform_.worldTranslate_;
	sphere.radius = radius_;
	MyMath::DrawSphere(sphere, debugColor_, debugLine_.get(), 10);
	debugLine_->Draw(camera);
#endif // _DEBUG
}

void EnemyBullet::UpdateParticle()
{
	if (isParticleActive) {
		particleTimer_++;

		for (int i = 0; i < kNumParticle_; i++) {
			//速度加算
			emitters_[i].transform.translate += particleVelocity_[i];
		}
		if (particleTimer_ >= kParticleTime_ - 150) {
			for (int i = 0; i < kNumParticle_; i++) {
				//パーティクル生成ストップ
				emitters_[i].count = 0;
			}
		}
		if (particleTimer_ >= kParticleTime_) {
			//パーティクル出したので死亡
			isDead_ = true;
		}
	}
}

void EnemyBullet::DrawParticle(const BaseCamera& camera)
{
	if (isParticleActive) {
		for (int i = 0; i < kNumParticle_; i++) {
			particles_[i]->Draw(camera, emitters_[i], &fields_[i]);
		}
	}
}

void EnemyBullet::OnCollision()
{
	//当たり判定属性をなしにする
	SetCollisionAttribute(kCollisionAttributeNothingness);

	//パーティクルをアクティブに
	isParticleActive = true;
	for (int i = 0; i < kNumParticle_; i++) {
		//ランダムエンジンの生成
		std::random_device seedGenerator;
		std::mt19937 randomEngine(seedGenerator());
		//エミッター
		emitters_[i].transform.scale = { 1.0f,1.0f,1.0f };
		emitters_[i].transform.rotate = { 0.0f,0.0f,0.0f };
		emitters_[i].transform.translate = worldTransform_.worldTranslate_;
		emitters_[i].count = 2;
		emitters_[i].frequency = 0.003f;
		emitters_[i].frequencyTime = 0.0f;
		//速度
		std::uniform_real_distribution<float> distVelocity(-1.0f, 1.0f);
		particleVelocity_[i] = { distVelocity(randomEngine),distVelocity(randomEngine),distVelocity(randomEngine) };
		particleVelocity_[i] *= 1.5f;
	}
	debugColor_ = { 1.0f,0.0f,0.0f,1.0f };

}

Vector3 EnemyBullet::GetWorldPosition()
{
	return worldTransform_.worldTranslate_;
}
