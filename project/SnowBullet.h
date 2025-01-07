#pragma once
#include "Collider.h"
#include "BaseCamera.h"
#include "SceneLight.h"
#include "WorldTransform.h"
#include "Object3d.h"
#include "LineDrawer.h"
#include "Audio.h"
#include <cstdint>
#include <memory>

class SnowBullet : public Collider
{
public:
	void Initialize();
	void Update();
	void Draw(const BaseCamera& camera, const SceneLight* light);
	void DrawLine(const BaseCamera& camera);

	void SetParent(const WorldTransform* parent) { worldTransform_.parent_ = parent; }
	void CancelParent() { worldTransform_.parent_ = nullptr; }
	void SetDirection(const Vector3& direction) { direction_ = direction; }

	bool GetIsDead() { return isDead_; }

public://公開メンバ変数
	bool isFire_;
	bool isMove_;
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
	Vector4 debugColor_ = { 0.0f,0.0f,1.0f,1.0f };

};

