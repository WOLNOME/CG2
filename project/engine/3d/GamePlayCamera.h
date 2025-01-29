#pragma once
#include "BaseCamera.h"

class GamePlayCamera : public BaseCamera
{
public:
	GamePlayCamera() : BaseCamera() {}
	virtual ~GamePlayCamera() = default;
	//初期化
	void Initialize() override;
	//更新
	void Update();

};

