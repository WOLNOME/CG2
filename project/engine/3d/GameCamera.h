#pragma once
#include "BaseCamera.h"
class GameCamera : public BaseCamera {
public:
	GameCamera() : BaseCamera() {}
	virtual ~GameCamera() = default;
	//初期化
	void Initialize() override;
	//更新
	void Update() override;

	//ImGui操作
	void DebugWithImGui();

	//カメラ演出系の関数
	void RegistShake(float time, float power = 0.5f);
	//シェイク中か
	bool GetIsShake();

private:
	//カメラの揺れの更新
	void UpdateShake();

private:
	// カメラの揺れを管理する構造体
	struct ShakeData {
		float maxTime;		// 揺れの最大時間
		float time;			// 揺れの時間
		float maxPower;		// 揺れの最大強さ
		float power;		// 揺れの強さ
	};
	// カメラの揺れを管理する変数
	std::list<ShakeData> shakeList_;
	Vector3 shakeOffset_ = { 0.0f,0.0f,0.0f };
};

