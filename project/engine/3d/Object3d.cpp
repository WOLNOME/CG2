#include "Object3d.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "MainRender.h"
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

	//リソース作成
	lightFlagResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(LightFlagForPS));
	//リソースにマッピング
	lightFlagResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightFlagData_));
	//データに書き込み
	lightFlagData_->isActiveLights = false;

}

void Object3d::Draw(WorldTransform& worldTransform, const  BaseCamera& camera, const SceneLight* sceneLight)
{
	//アニメーション反映処理
	model_->Update();

	//使用するGPSを選択
	if (model_->IsAnimation()) {
		Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::Animation);
	}
	else if (!model_->IsAnimation()) {
		Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::None);
	}

	//シーンライト有無設定
	lightFlagData_->isActiveLights = (sceneLight != nullptr) ? true : false;

	//lightFlagCbufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(6, lightFlagResource_->GetGPUVirtualAddress());

	//SceneLightCBufferの場所を設定
	if (lightFlagData_->isActiveLights) {
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, sceneLight->GetSceneLightConstBuffer()->GetGPUVirtualAddress());
	}

	//WorldTransformCBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());

	//CameraからビュープロジェクションCBufferの場所設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());

	//Cameraからカメラ座標CBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, camera.GetCameraPositionConstBuffer()->GetGPUVirtualAddress());

	//モデルを描画する
	model_->Draw(0, 3);
}
