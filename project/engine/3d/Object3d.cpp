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

void Object3d::Initialize(const std::string& filePath)
{
	//モデルマネージャーでモデルを生成
	ModelManager::GetInstance()->LoadModel(filePath);
	//モデルマネージャーから検索してセットする
	model_ = ModelManager::GetInstance()->FindModel(filePath);

	//光源有無リソース作成
	lightFlagResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(LightFlagForPS));
	//リソースをマッピング
	lightFlagResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightFlagData_));
	//データに書き込み
	lightFlagData_->isDirectionalLight = false;

}

void Object3d::Draw(const WorldTransform& worldTransform, const BaseCamera& camera, const DirectionalLight* dirLight)
{
	//平行光源有無の設定
	lightFlagData_->isDirectionalLight = (dirLight && dirLight->isActive_) ? true : false;
	

	//平行光源のCBufferの場所を設定
	if (dirLight && dirLight->isActive_)
		DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, dirLight->GetConstBuffer()->GetGPUVirtualAddress());

	//光源有無確認用CBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, lightFlagResource_->GetGPUVirtualAddress());

	//WorldTransformCBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());

	//CameraからビュープロジェクションCBufferの場所設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());

	//Cameraからカメラ座標CBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(6, camera.GetCameraPositionConstBuffer()->GetGPUVirtualAddress());

	//モデルを描画する
	model_->Draw(0, 3);
}


