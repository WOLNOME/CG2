#pragma once
#include <list>
#include "Collider.h"

class CollisionManager {
private://コンストラクタ等の隠蔽
	static CollisionManager* instance;
	CollisionManager() = default;//コンストラクタ隠蔽
	~CollisionManager() = default;//デストラクタ隠蔽
	CollisionManager(CollisionManager&) = delete;//コピーコンストラクタ封印
	CollisionManager& operator=(CollisionManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static CollisionManager* GetInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();

public:
	// 当たり判定チェック
	void CheckCollision();
	// コライダー登録関数
	void SetColliders(Collider* colliders);
	// コライダーリストクリア関数
	void ClearColliders() { colliders_.clear(); }

private:
	// 当たり判定演算
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);
	

private://メンバ変数
	// コライダーリスト
	std::list<Collider*> colliders_;
};
