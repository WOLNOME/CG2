#include "Enemy.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "CollisionConfig.h"
#include <numbers>

void Enemy::Initialize()
{
	//ワールドトランスフォームの初期化
	worldTransform_.Initialize();
	worldTransform_.translate_ = { 0.0f,18.0f,80.0f };
	worldTransform_.rotate_ = { 0.0f,std::numbers::pi_v<float>,0.0f };
	worldTransform_.scale_ = { 8.0f,8.0f,8.0f };
	//モデルの生成と初期化
	model_ = std::make_unique<Object3d>();
	model_->InitializeModel("winterGeneral", GLTF);
	//UIの生成と初期化
	textureHandleRedBar_ = TextureManager::GetInstance()->LoadTexture("redBar.png");
	textureHandleGreenBar_ = TextureManager::GetInstance()->LoadTexture("greenBar.png");
	spriteRedBar_ = std::make_unique<Sprite>();
	spriteGreenBar_ = std::make_unique<Sprite>();
	spriteRedBar_->Initialize(textureHandleRedBar_);
	spriteGreenBar_->Initialize(textureHandleGreenBar_);
	spriteRedBar_->SetPosition({ 270.0f,32.0f });
	spriteGreenBar_->SetPosition({ 270.0f,32.0f });
	greenBarSize = spriteGreenBar_->GetSize();

	//コライダー関連の初期化
	SetCollisionAttribute(kCollisionAttributeEnemy);
	radius_ = 15.2f;
	debugLine_ = std::make_unique<LineDrawer>();
	debugLine_->Initialize();

	//効果音
	audio_ = std::make_unique<Audio>();
	audio_->Initialize("breakSnow.wav");

	//弾の生成
	bullet_ = std::make_unique<EnemyBullet>();
}

void Enemy::Update(const Vector3& playerPosition)
{
	//ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();

	//死亡処理
	if (hp_ <= 0) {
		isDead_ = true;
	}

	//スプライトの更新処理
	spriteRedBar_->Update();
	spriteGreenBar_->Update();

	//HPに応じて緑バーを縮小させる
	spriteGreenBar_->SetSize({ greenBarSize.x * (hp_ / 100.0f),greenBarSize.y });

	//攻撃に関する処理
	Attack(playerPosition);


#ifdef _DEBUG
	ImGui::Begin("enemy");
	ImGui::DragFloat3("translate", &worldTransform_.translate_.x, 0.1f);
	ImGui::DragFloat3("rotate", &worldTransform_.rotate_.x, 0.1f);
	ImGui::DragFloat3("scale", &worldTransform_.scale_.x, 0.1f);
	ImGui::Text("HP : %d", hp_);
	ImGui::DragFloat("radius", &radius_, 0.1f);
	ImGui::End();

	debugColor_ = { 0.0f,0.0f,1.0f,1.0f };
#endif // _DEBUG

}

void Enemy::Draw(const BaseCamera& camera, const SceneLight* light)
{
	//モデルの描画
	model_->Draw(worldTransform_, camera, light);
	//弾の描画
	if (bullet_ && isAttack_) {
		bullet_->Draw(camera, light);
	}
}

void Enemy::DrawLine(const BaseCamera& camera)
{
#ifdef _DEBUG
	Sphere sphere;
	sphere.center = worldTransform_.worldTranslate_;
	sphere.radius = radius_;
	MyMath::DrawSphere(sphere, debugColor_, debugLine_.get(), 10);
	debugLine_->Draw(camera);
	if (bullet_ && isAttack_) {
		bullet_->DrawLine(camera);
	}
#endif // _DEBUG

}

void Enemy::DrawParticle(const BaseCamera& camera)
{
	if (bullet_ && isAttack_) {
		bullet_->DrawParticle(camera);
	}
}

void Enemy::DrawSprite()
{
	spriteRedBar_->Draw();
	spriteGreenBar_->Draw();
}

void Enemy::Attack(const Vector3& playerPosition)
{
	//ローカル変数
	float t = 0.0f;

	timer_++;
	//4秒後に攻撃準備
	if (timer_ == readyTime_) {
		isReady_ = true;
	}
	//6秒後に攻撃
	else if (timer_ == attackTime_) {
		isReady_ = false;
		isAttack_ = true;
		//弾の初期化
		if (bullet_) {
			bullet_->Initialize(worldTransform_.worldTranslate_, playerPosition);
		}
	}
	//8秒後にリセット
	else if (timer_ >= endTime_) {
		timer_ = 0;
		worldTransform_.rotate_.x = 0.0f;
		isAttack_ = false;
	}

	//攻撃準備処理
	if (isReady_) {
		int readyTimer = timer_ - readyTime_;
		t = readyTimer / float(attackTime_ - readyTime_);
		worldTransform_.rotate_.x = MyMath::Lerp(0.0f, 1.0f, MyMath::EaseInCubic(t));
	}
	//攻撃処理
	if (isAttack_) {
		int attackTimer = timer_ - attackTime_;
		t = attackTimer / float(endTime_ - attackTime_);
		worldTransform_.rotate_.x = MyMath::Lerp(1.0f, -std::numbers::pi_v<float>*2.0f * 3, MyMath::EaseOutCubic(t));

		if (bullet_) {
			//弾の死亡処理
			if (bullet_->GetIsDead()) {
				bullet_.reset();
				//直ちに新しい弾を生成
				bullet_ = std::make_unique<EnemyBullet>();
			}
			//弾の更新
			bullet_->Update();
		}

	}
}

void Enemy::OnCollision()
{
	hp_ -= 10;
	debugColor_ = { 1.0f,0.0f,0.0f,1.0f };
	audio_->Play(false, 1.2f);

}

Vector3 Enemy::GetWorldPosition()
{
	return worldTransform_.worldTranslate_;
}