#pragma once
#include "BaseCamera.h"
#include "Vector3.h"
#include "Vector4.h"
#include "CollisionConfig.h"
#include "LineDrawer.h"
class Collider {
public:
	//デストラクタ
	virtual ~Collider() = default;

	//当たり判定時の処理
	virtual void OnCollision(CollisionAttribute attribute) = 0;

public://当たり判定可視化用ラインに関する関数群
	//デバッグライン初期化
	void InitDebugLine();
	//当たり判定可視化用ラインの描画
	void DrawCollisionLine(BaseCamera* _camera);

public://getter
	//半径を取得
	float GetRadius() { return radius_; }
	//ワールド座標を取得
	virtual Vector3 GetWorldPosition() = 0;
	//属性の取得
	CollisionAttribute GetCollisionAttribute() { return collisionAttribute_; }

public://setter
	//属性のセット
	void SetCollisionAttribute(const CollisionAttribute collisionAttribute) { collisionAttribute_ = collisionAttribute; }

private:
	CollisionAttribute collisionAttribute_ = CollisionAttribute::Nothingness;
	//デバッグ用ライン
	std::unique_ptr<LineDrawer> lineDrawer_ = nullptr;
protected:
	float radius_ = 1.0f;
	Vector4 debugLineColor_ = { 1.0f,1.0f,1.0f,1.0f };
};
