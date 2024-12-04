#include "Camera.h"
#include "WinApp.h"

Camera::Camera()
//デフォルトコンストラクタ（作った時点で初期化される→Initializeの手間を省く）
	: transform({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} })
	, fovY(0.45f)
	, aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f)
	, worldMatrix(MakeAffineMatrix(transform.scale, transform.rotate, transform.translate))
	, viewMatrix(Inverse(worldMatrix))
	, projectionMatrix(MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip))
	, viewProjectionMatrix(Multiply(viewMatrix, projectionMatrix))
{}

void Camera::Update()
{
	//トランスフォームからアフィン変換行列を計算
	worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	//worldMatrixの逆行列
	viewMatrix = Inverse(worldMatrix);
	//透視投影行列の生成
	projectionMatrix = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	//合成行列の生成
	viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);

}
