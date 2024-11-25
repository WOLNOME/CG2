#include "Object3d.h"
#include "DirectXCommon.h"
#include "Object3dCommon.h"
#include "fstream"
#include "sstream"
#include "TextureManager.h"
#include "ModelManager.h"
#include "WinApp.h"
#include "BaseCamera.h"
#include <cassert>

void Object3d::Initialize(const std::string& filePath, ModelFormat format)
{
	//モデルマネージャーでモデルを生成
	ModelManager::GetInstance()->LoadModel(filePath, format);
	//モデルマネージャーから検索してセットする
	model_ = ModelManager::GetInstance()->FindModel(filePath);

}

void Object3d::Draw(const WorldTransform& worldTransform, const BaseCamera& camera, const SceneLight* sceneLight)
{
	//SceneLightCBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, sceneLight->GetSceneLightConstBuffer()->GetGPUVirtualAddress());

	//WorldTransformCBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());

	//CameraからビュープロジェクションCBufferの場所設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());

	//Cameraからカメラ座標CBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, camera.GetCameraPositionConstBuffer()->GetGPUVirtualAddress());

	//モデルを描画する
	model_->Draw(0, 3);
}

void Object3d::DrawShadow(const WorldTransform& worldTransform, const SceneLight* sceneLight)
{
	//WorldTransformCBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());
	
	//SceneLightCBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, sceneLight->GetLightViewProjectionConstBuffer()->GetGPUVirtualAddress());

	//シャドウマップ生成用のモデルを描画
	model_->DrawShadow();
}

