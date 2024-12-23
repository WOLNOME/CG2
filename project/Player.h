#pragma once
#include "WorldTransform.h"
#include "Object3d.h"
#include "BaseCamera.h"
#include "SceneLight.h"
#include "Input.h"
#include "MyMath.h"
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

private:
	void Move();

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

};

