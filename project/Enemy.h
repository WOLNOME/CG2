#pragma once
#include "Collider.h"
#include "Sprite.h"
#include "Audio.h"
#include "WorldTransform.h"
#include "BaseCamera.h"
#include "SceneLight.h"
#include "Object3d.h"
#include "LineDrawer.h"
#include "EnemyBullet.h"
#include <cstdint>
#include <memory>
class Enemy : public Collider
{
public:
	void Initialize();
	void Update(const Vector3& playerPosition);
	void Draw(const BaseCamera& camera, const SceneLight* light);
	void DrawLine(const BaseCamera& camera);
	void DrawParticle(const BaseCamera& camera);
	void DrawSprite();

	const std::unique_ptr<EnemyBullet>& GetBullet() { return bullet_; }
	bool GetIsDead() { return isDead_; }
private:
	void Attack(const Vector3& playerPosition);

public://コライダー関連
	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision() override;
	// ワールド座標を取得
	Vector3 GetWorldPosition() override;
private:
	//ワールドトランスフォーム
	WorldTransform worldTransform_;
	//モデル
	std::unique_ptr<Object3d> model_ = nullptr;
	//HP
	int32_t hp_ = 100;
	//死亡判定
	bool isDead_ = false;

	//HPバー
	uint32_t textureHandleRedBar_ = 0u;
	uint32_t textureHandleGreenBar_ = 0u;
	std::unique_ptr<Sprite> spriteRedBar_ = nullptr;
	std::unique_ptr<Sprite> spriteGreenBar_ = nullptr;
	Vector2 greenBarSize;

	//デバッグ用線
	std::unique_ptr<LineDrawer> debugLine_ = nullptr;
	Vector4 debugColor_ = { 0.0f,0.0f,1.0f,1.0f };

	//効果音
	std::unique_ptr<Audio> audio_ = nullptr;

	//攻撃に関する変数
	std::unique_ptr<EnemyBullet> bullet_ = nullptr;
	int timer_ = 0;
	const int readyTime_ = 240;
	const int attackTime_ = 360;
	const int endTime_ = 540;
	bool isReady_ = false;
	bool isAttack_ = false;


};

