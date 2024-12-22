#pragma once
#include "BaseCamera.h"
#include "SceneLight.h"
#include "WorldTransform.h"
#include "Object3d.h"
#include <memory>

class Skydome
{
public:
	Skydome();
	~Skydome();

	void Initialize();
	void Update();
	void Draw(const BaseCamera& camera, const SceneLight* light = nullptr);

private:
	//ワールドトランスフォーム
	WorldTransform worldTransform_;
	//モデル
	std::unique_ptr<Object3d> model_ = nullptr;

};

