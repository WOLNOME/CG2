#include "Camera.h"
#include "WinApp.h"
#include "DirectXCommon.h"

Camera::Camera()
//デフォルトコンストラクタ（作った時点で初期化される→Initializeの手間を省く）
	: transform({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} })
	, fovY(0.45f)
	, aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f)
	, worldMatrix(MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate))
	, viewMatrix(MyMath::Inverse(worldMatrix))
	, projectionMatrix(MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip))
	, viewProjectionMatrix(MyMath::Multiply(viewMatrix, projectionMatrix))
{}

void Camera::Initialize()
{
	//リソースの作成
	resource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ViewProjectionTransformationMatrixForVS));
	//リソースをマッピング
	resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	//データに書き込み
	data_->matView = viewMatrix;
	data_->matProjection = projectionMatrix;
	data_->worldCameraPos = { worldMatrix.m[3][0],worldMatrix.m[3][1],worldMatrix.m[3][2] };
}

void Camera::UpdateMatrix()
{
	//トランスフォームからアフィン変換行列を計算
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	//worldMatrixの逆行列
	viewMatrix = MyMath::Inverse(worldMatrix);
	//透視投影行列の生成
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	//合成行列の生成
	viewProjectionMatrix = MyMath::Multiply(viewMatrix, projectionMatrix);

	//データの転送
	data_->matView = viewMatrix;
	data_->matProjection = projectionMatrix;
	data_->worldCameraPos = { worldMatrix.m[3][0],worldMatrix.m[3][1],worldMatrix.m[3][2],1.0f };
}
