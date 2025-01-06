#pragma once
#include "Collider.h"
#include "Sprite.h"
#include "WorldTransform.h"
#include "BaseCamera.h"
#include "SceneLight.h"
#include "Object3d.h"
#include "LineDrawer.h"
#include <cstdint>
#include <memory>
class Enemy : public Collider
{
public:
	void Initialize();
	void Update();
	void Draw(const BaseCamera& camera, const SceneLight* light);
	void DrawLine(const BaseCamera& camera);
	void DrawSprite();

	bool GetIsDead() { return isDead_; }
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
};

