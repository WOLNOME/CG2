#include "BaseCamera.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include <vector>
#include <random>

BaseCamera::BaseCamera()
	: transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} })
	, fovY(0.45f)
	, aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f)
	, standardPosition({ 0.0f, 0.0f, 0.0f })
	, shakeOffset_({ 0.0f, 0.0f, 0.0f }) {
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = MyMath::Inverse(worldMatrix);
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	viewProjectionMatrix = MyMath::Multiply(viewMatrix, projectionMatrix);
}

void BaseCamera::Initialize() {
	//座標変換用リソース
	cameraInfoResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(CameraInfoForVS));
	cameraInfoResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraInfoData_));
	cameraInfoData_->matWorld = worldMatrix;
	cameraInfoData_->matView = viewMatrix;
	cameraInfoData_->matProjection = projectionMatrix;
	//カメラ座標リソース
	cameraPositionResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(CameraInfoForVS));
	cameraPositionResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraPositionData_));
	cameraPositionData_->worldPosition = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2] };
}

void BaseCamera::UpdateMatrix() {
	//揺れ幅の更新
	UpdateShake();
	//座標を求める
	transform.translate = standardPosition + shakeOffset_;

	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = MyMath::Inverse(worldMatrix);
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	viewProjectionMatrix = MyMath::Multiply(viewMatrix, projectionMatrix);

	cameraInfoData_->matWorld = worldMatrix;
	cameraInfoData_->matView = viewMatrix;
	cameraInfoData_->matProjection = projectionMatrix;

	cameraPositionData_->worldPosition = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2] };
}

void BaseCamera::DebugWithImGui() {
#ifdef _DEBUG

	ImGui::Begin("Camera");
	ImGui::DragFloat3("Translate", &standardPosition.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.01f);
	ImGui::End();

#endif // _DEBUG
}

const Vector3 BaseCamera::GetForwardDirection() {
	// オイラー角から回転行列を計算（X軸、Y軸、Z軸回転を順番に適用）
	Matrix4x4 rotationMatrix = MyMath::CreateRotationFromEulerAngles(transform.rotate.x, transform.rotate.y, transform.rotate.z);

	// 回転行列を使って前方向ベクトルを計算（Z軸方向が前方向）
	Vector3 forward = rotationMatrix * Vector3(0, 0, 1);  // Z軸方向を前方向として扱う

	return forward;
}

const Vector3 BaseCamera::GetRightDirection() {
	// オイラー角から回転行列を計算（X軸、Y軸、Z軸回転を順番に適用）
	Matrix4x4 rotationMatrix = MyMath::CreateRotationFromEulerAngles(transform.rotate.x, transform.rotate.y, transform.rotate.z);

	// 回転行列を使って前方向ベクトルを計算（Z軸方向が前方向）
	Vector3 forward = rotationMatrix * Vector3(1, 0, 0);  // Z軸方向を前方向として扱う

	return forward;
}

const Vector3 BaseCamera::GetLeftDirection() {
	// オイラー角から回転行列を計算（X軸、Y軸、Z軸回転を順番に適用）
	Matrix4x4 rotationMatrix = MyMath::CreateRotationFromEulerAngles(transform.rotate.x, transform.rotate.y, transform.rotate.z);

	// 回転行列を使って前方向ベクトルを計算（Z軸方向が前方向）
	Vector3 forward = rotationMatrix * Vector3(-1, 0, 0);  // Z軸方向を前方向として扱う

	return forward;
}

const Vector3 BaseCamera::GetBackDirection() {
	// オイラー角から回転行列を計算（X軸、Y軸、Z軸回転を順番に適用）
	Matrix4x4 rotationMatrix = MyMath::CreateRotationFromEulerAngles(transform.rotate.x, transform.rotate.y, transform.rotate.z);

	// 回転行列を使って前方向ベクトルを計算（Z軸方向が前方向）
	Vector3 forward = rotationMatrix * Vector3(0, 0, -1);  // Z軸方向を前方向として扱う

	return forward;
}

void BaseCamera::RegistShake(float time, float power) {
	//揺れのデータをとる
	ShakeData shakeData;
	shakeData.maxTime = time;
	shakeData.time = time;
	shakeData.maxPower = power;
	shakeData.power = power;
	//リストに登録
	shakeList_.push_back(shakeData);
}

void BaseCamera::UpdateShake() {
	//オフセットを0で更新
	shakeOffset_ = { 0.0f, 0.0f, 0.0f };
	//リストに何もなければ終了
	if (shakeList_.empty()) return;
	//ローカル変数
	float usePower = 0.0f;
	//全てのリストを更新
	for (auto it = shakeList_.begin(); it != shakeList_.end();) {
		//揺れの大きさを線形補完で決める
		it->power = MyMath::Lerp(it->maxPower, 0.0f, 1.0f - (it->time / it->maxTime));
		//時間を減らす
		it->time -= kDeltaTime;
		//時間が0未満になったら削除
		if (it->time < 0.0f) {
			it = shakeList_.erase(it);
			//次の要素へ
			continue;
		}
		//揺れの大きさが大きいほうを使う
		if (usePower < it->power) {
			usePower = it->power;
		}
		//次の要素へ
		it++;
	}
	//最終的に決まった揺れの大きさを使ってオフセットを決める
	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());
	std::uniform_real_distribution<float> dist(-usePower, usePower);
	shakeOffset_.x = dist(engine);
	shakeOffset_.y = dist(engine);
	shakeOffset_.z = dist(engine);
}
