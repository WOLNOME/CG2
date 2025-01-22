#pragma once
#include "WorldTransform.h"
#include "Object3d.h"
#include "BaseCamera.h"
#include "SceneLight.h"
#include "Input.h"
#include "MyMath.h"
#include "SnowBullet.h"
#include <memory>
#include <numbers>

class Player
{
public:
	Player();
	~Player();

	void Initialize();
	void Update();
	void Draw(const BaseCamera& camera, const SceneLight* light = nullptr);
	void DrawLine(const BaseCamera& camera);
	void DrawParticle(const BaseCamera& camera);

	const std::unique_ptr<SnowBullet>& GetBullet() { return snowBullet_; }
private:
	//移動
	void Move();
	//発射
	void Fire();
	//弾
	void Bullet();

private:
	//インプット
	Input* input_ = nullptr;
	//ワールドトランスフォーム
	WorldTransform worldTransform_;
	//モデル
	std::unique_ptr<Object3d> model_ = nullptr;

	//移動系のメンバ変数
	const float pi = std::numbers::pi_v<float>;
	const float speed_ = 0.2f;
	const float rotateSpeed_ = (5.0f / 180.0f) * pi;

	//発射系のメンバ変数
	std::unique_ptr<SnowBullet> snowBullet_ = nullptr;
	bool isFire_ = false;

};

