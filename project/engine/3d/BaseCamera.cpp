#include "BaseCamera.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include <vector>

BaseCamera::BaseCamera()
	: fovY(0.45f)
	, aspectRatio(float(WinApp::kClientWidth) / float(WinApp::kClientHeight))
	, nearClip(0.1f)
	, farClip(100.0f){
	worldTransform.Initialize();
	viewMatrix = MyMath::Inverse(worldTransform.matWorld);
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	viewProjectionMatrix = MyMath::Multiply(viewMatrix, projectionMatrix);
}

void BaseCamera::Initialize() {
	//座標変換用リソース
	viewProjectionResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ViewProjectionTransformationMatrixForVS));
	viewProjectionResource_->Map(0, nullptr, reinterpret_cast<void**>(&viewProjectionData_));
	viewProjectionData_->matWorld = worldTransform.matWorld;
	viewProjectionData_->matView = viewMatrix;
	viewProjectionData_->matProjection = projectionMatrix;
	//カメラ座標リソース
	cameraPositionResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ViewProjectionTransformationMatrixForVS));
	cameraPositionResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraPositionData_));
	cameraPositionData_->worldPosition = worldTransform.worldTranslate;
}

void BaseCamera::Update() {
	//カメラの更新
	UpdateMatrix();
}

void BaseCamera::UpdateMatrix() {
	//ワールドトランスフォームの更新
	worldTransform.UpdateMatrix();

	viewMatrix = MyMath::Inverse(worldTransform.matWorld);
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);
	viewProjectionMatrix = MyMath::Multiply(viewMatrix, projectionMatrix);

	viewProjectionData_->matWorld = worldTransform.matWorld;
	viewProjectionData_->matView = viewMatrix;
	viewProjectionData_->matProjection = projectionMatrix;

	cameraPositionData_->worldPosition = worldTransform.worldTranslate;
}

void BaseCamera::DebugWithImGui() {
#ifdef _DEBUG

	ImGui::Begin("Camera");
	ImGui::DragFloat3("Translate", &worldTransform.translate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &worldTransform.rotate.x, 0.01f);
	ImGui::End();

#endif // _DEBUG
}
