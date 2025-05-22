#include "CollisionManager.h"
#include "CollisionConfig.h"
#include "MyMath.h"

void CollisionManager::CheckCollision() {
	// リスト内のペアを総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {
		// イテレータAからコライダーAを取得する
		Collider* colliderA = *itrA;
		// イテレータBはイテレーターAの次の要素から回す
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;
		for (; itrB != colliders_.end(); ++itrB) {
			// イテレーターBからコライダーBを取得する
			Collider* colliderB = *itrB;
			// 衝突フィルタリング
			// 自分と同じ属性だと当たらない
			if (colliderA->GetCollisionAttribute() == colliderB->GetCollisionAttribute()) {
				continue;
			}
			// 自機と自弾&必殺弾
			if (colliderA->GetCollisionAttribute() == kCollisionAttributePlayer) {
				if (colliderB->GetCollisionAttribute() == kCollisionAttributePlayerBullet) {
					continue;
				}
			}
			if (colliderB->GetCollisionAttribute() == kCollisionAttributePlayer) {
				if (colliderA->GetCollisionAttribute() == kCollisionAttributePlayerBullet) {
					continue;
				}
			}
			// 敵と敵弾
			if (colliderA->GetCollisionAttribute() == kCollisionAttributeEnemy) {
				if (colliderB->GetCollisionAttribute() == kCollisionAttributeEnemyBullet) {
					continue;
				}
			}
			if (colliderB->GetCollisionAttribute() == kCollisionAttributeEnemy) {
				if (colliderA->GetCollisionAttribute() == kCollisionAttributeEnemyBullet) {
					continue;
				}
			}
			// 虚無オブジェクトは当たり判定を取らない
			if (colliderA->GetCollisionAttribute() == kCollisionAttributeNothingness) {
				if (colliderB->GetCollisionAttribute() > 0) {
					continue;
				}
			}
			if (colliderB->GetCollisionAttribute() == kCollisionAttributeNothingness) {
				if (colliderA->GetCollisionAttribute() > 0) {
					continue;
				}
			}

			// ペアの当たり判定
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {
	Vector3 posA, posB;
	// Aの座標
	posA = colliderA->GetWorldPosition();
	// Bの座標
	posB = colliderB->GetWorldPosition();
	// ABの差
	Vector3 AtoB = posA - posB;
	// 半径の和
	float addRad = colliderA->GetRadius() + colliderB->GetRadius();
	// 座標AとBの距離を求める
	float length = AtoB.Length();
	// 球と球の交差判定
	if (length < addRad) {
		// 衝突時コールバックを呼び出す
		colliderA->OnCollision();
		colliderB->OnCollision();
	}
}
