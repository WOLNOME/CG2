#pragma once
#include "Collider.h"
#include "BaseCamera.h"
#include "SceneLight.h"
#include "WorldTransform.h"
#include "Object3d.h"
#include "LineDrawer.h"
#include "Audio.h"
#include "Particle.h"
#include <cstdint>
#include <list>
#include <memory>

class EnemyBullet : public Collider
{
public:
	EnemyBullet();

	void Initialize(const Vector3& enemyPosition, const Vector3& playerPosition);
	void Update();
	void Draw(const BaseCamera& camera, const SceneLight* light);
	void DrawLine(const BaseCamera& camera);
	void DrawParticle(const BaseCamera& camera);

	bool GetIsDead() { return isDead_; }

private:
	void UpdateParticle();

public://コライダー関連
	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision() override;
	// ワールド座標を取得
	Vector3 GetWorldPosition() override;
private:
	//ワールドトランスフォーム
	WorldTransform worldTransform_;
	//モデル
	uint32_t textureHandle_;
	std::unique_ptr<Object3d> model_ = nullptr;
	//必要メンバ変数
	float speed_;
	Vector3 direction_;
	bool isDead_;

	//デバッグ用の線描画
	std::unique_ptr<LineDrawer> debugLine_ = nullptr;
	Vector4 debugColor_;

	//パーティクル
	static const int kNumParticle_ = 10;
	static const int kParticleTime_ = 160;
	int particleTimer_;
	bool isParticleActive;
	std::array<Vector3, kNumParticle_> particleVelocity_;
	std::array<std::unique_ptr<Particle>, kNumParticle_> particles_;
	std::array<Particle::Emitter, kNumParticle_ > emitters_;
	std::array<Particle::AccelerationField, kNumParticle_ > fields_;


};

