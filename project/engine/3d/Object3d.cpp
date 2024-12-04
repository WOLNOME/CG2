#include "Object3d.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "ShadowMapRender.h"
#include "MainRender.h"
#include "ShadowMapManager.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "ModelManager.h"
#include "BaseCamera.h"
#include <fstream>
#include <sstream>
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
	//SLシャドウマップテクスチャのデータを送る
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(6, SrvManager::GetInstance()->GetGPUDescriptorHandle(ShadowMapManager::GetInstance()->GetSLSMInfo().srvIndex));

	//SceneLightCBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, sceneLight->GetSceneLightConstBuffer()->GetGPUVirtualAddress());

	//WorldTransformCBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());

	//CameraからビュープロジェクションCBufferの場所設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());

	//Cameraからカメラ座標CBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, camera.GetCameraPositionConstBuffer()->GetGPUVirtualAddress());

	//モデルを描画する
	model_->Draw(0, 3);
}

void Object3d::DrawShadow(const WorldTransform& worldTransform, const SceneLight* sceneLight, uint32_t lVPMIndex)
{
	//WorldTransformCBufferの場所を設定
	ShadowMapRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());
	
	//SceneLightCBufferの場所を設定
	ShadowMapRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, ShadowMapManager::GetInstance()->GetLightViewProjectionConstBuffer(lVPMIndex)->GetGPUVirtualAddress());

	//シャドウマップ生成用のモデルを描画
	model_->DrawShadow();
}

