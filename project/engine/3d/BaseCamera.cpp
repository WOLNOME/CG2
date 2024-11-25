#include "BaseCamera.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include <vector>

BaseCamera::BaseCamera()
	: transform({ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} })
	, fovY(0.45f)
	, aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f)
{
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = MyMath::Inverse(worldMatrix);
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	viewProjectionMatrix = MyMath::Multiply(viewMatrix, projectionMatrix);
}

void BaseCamera::Initialize()
{
	//座標変換用リソース
	viewProjectionResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ViewProjectionTransformationMatrixForVS));
	viewProjectionResource_->Map(0, nullptr, reinterpret_cast<void**>(&viewProjectionData_));
	viewProjectionData_->matView = viewMatrix;
	viewProjectionData_->matProjection = projectionMatrix;
	//カメラ座標リソース
	cameraPositionResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ViewProjectionTransformationMatrixForVS));
	cameraPositionResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraPositionData_));
	cameraPositionData_->worldPosition = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2] };
}

void BaseCamera::UpdateMatrix()
{
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = MyMath::Inverse(worldMatrix);
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	viewProjectionMatrix = MyMath::Multiply(viewMatrix, projectionMatrix);

	viewProjectionData_->matView = viewMatrix;
	viewProjectionData_->matProjection = projectionMatrix;

	cameraPositionData_->worldPosition = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2] };
}

const Vector3 BaseCamera::GetForwardDirection()
{
	// オイラー角から回転行列を計算（X軸、Y軸、Z軸回転を順番に適用）
	Matrix4x4 rotationMatrix = MyMath::CreateRotationFromEulerAngles(transform.rotate.x, transform.rotate.y, transform.rotate.z);

	// 回転行列を使って前方向ベクトルを計算（Z軸方向が前方向）
	Vector3 forward = rotationMatrix * Vector3(0, 0, 1);  // Z軸方向を前方向として扱う

	return forward;
}

const Vector3 BaseCamera::GetRightDirection()
{
	// オイラー角から回転行列を計算（X軸、Y軸、Z軸回転を順番に適用）
	Matrix4x4 rotationMatrix = MyMath::CreateRotationFromEulerAngles(transform.rotate.x, transform.rotate.y, transform.rotate.z);

	// 回転行列を使って前方向ベクトルを計算（Z軸方向が前方向）
	Vector3 forward = rotationMatrix * Vector3(1, 0, 0);  // Z軸方向を前方向として扱う

	return forward;
}

const Vector3 BaseCamera::GetLeftDirection()
{
	// オイラー角から回転行列を計算（X軸、Y軸、Z軸回転を順番に適用）
	Matrix4x4 rotationMatrix = MyMath::CreateRotationFromEulerAngles(transform.rotate.x, transform.rotate.y, transform.rotate.z);

	// 回転行列を使って前方向ベクトルを計算（Z軸方向が前方向）
	Vector3 forward = rotationMatrix * Vector3(-1, 0, 0);  // Z軸方向を前方向として扱う

	return forward;
}

const Vector3 BaseCamera::GetBackDirection()
{
	// オイラー角から回転行列を計算（X軸、Y軸、Z軸回転を順番に適用）
	Matrix4x4 rotationMatrix = MyMath::CreateRotationFromEulerAngles(transform.rotate.x, transform.rotate.y, transform.rotate.z);

	// 回転行列を使って前方向ベクトルを計算（Z軸方向が前方向）
	Vector3 forward = rotationMatrix * Vector3(0, 0, -1);  // Z軸方向を前方向として扱う

	return forward;
}
