#include "SpotLight.h"
#include "DirectXCommon.h"
#include "MyMath.h"

void SpotLight::Update()
{
	//ライトの余弦をフォールオフ開始角度より大きくしないようにする処理
	if (cosFalloffStart_ < cosAngle_) {
		cosAngle_ = cosFalloffStart_ - 0.01f;
	}

	// ライト視点のビュー行列を計算
	Vector3 up = { 0.0f, 1.0f, 0.0f }; // ライトの上方向（必要に応じて変更）
	Vector3 target = position_ + direction_.Normalized(); // 注視点（ライトの方向）
	Matrix4x4 view = MyMath::LookAt(position_, target, up);

	// プロジェクション行列を計算
	float fovY = acosf(cosAngle_) * 2.0f; // スポットライトの視野角を計算
	float aspect = 1.0f; // アスペクト比（必要に応じて変更）
	float nearClip = 0.1f; // 近クリップ距離
	float farClip = distance_; // 遠クリップ距離
	Matrix4x4 projection = MyMath::MakePerspectiveFovMatrix(fovY, aspect, nearClip, farClip);

	// ビュープロジェクション行列を計算
	data_.viewProjection = projection * view;

	

	data_.color = color_;
	data_.position = position_;
	data_.intensity = intencity_;
	data_.direction = direction_.Normalized();
	data_.distance = distance_;
	data_.decay = decay_;
	data_.cosAngle = cosAngle_;
	data_.cosFalloffStart = cosFalloffStart_;
	data_.isActive = isActive_;
}

