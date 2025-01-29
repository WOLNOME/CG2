#pragma once
#include <list>
#include "Collider.h"

class CollisionManager {
public:
	// 当たり判定チェック
	void CheckCollision();
	// コライダーリスト登録関数
	void SetColliders(std::list<Collider*> colliders) { colliders_ = colliders; }
	// コライダーリストクリア関数
	void ClearColliders() { colliders_.clear(); }

private:
	// 当たり判定演算
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);
	
	// コライダーリスト
	std::list<Collider*> colliders_;
};
