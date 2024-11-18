#include "BaseCamera.h"
#include "WinApp.h"
#include "DirectXCommon.h"

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
	resource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ViewProjectionTransformationMatrixForVS));
	resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	data_->matView = viewMatrix;
	data_->matProjection = projectionMatrix;
	data_->worldCameraPos = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2], 1.0f };
}

void BaseCamera::UpdateMatrix()
{
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = MyMath::Inverse(worldMatrix);
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	viewProjectionMatrix = MyMath::Multiply(viewMatrix, projectionMatrix);

	data_->matView = viewMatrix;
	data_->matProjection = projectionMatrix;
	data_->worldCameraPos = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2], 1.0f };
}

const Vector3 BaseCamera::GetForwardDirection()
{
	// オイラー角から回転行列を計算（X軸、Y軸、Z軸回転を順番に適用）
	Matrix4x4 rotationMatrix = MyMath::CreateRotationFromEulerAngles(transform.rotate.x, transform.rotate.y, transform.rotate.z);

	// 回転行列を使って前方向ベクトルを計算（Z軸方向が前方向）
	Vector3 forward = rotationMatrix * Vector3(0, 0, 1);  // Z軸方向を前方向として扱う

	return forward;
}
