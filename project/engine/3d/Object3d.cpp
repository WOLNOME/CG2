#include "Object3d.h"
#include "DirectXCommon.h"
#include "Object3dCommon.h"
#include "fstream"
#include "sstream"
#include "TextureManager.h"
#include "ModelManager.h"
#include "WinApp.h"
#include "Camera.h"
#include <cassert>

void Object3d::Initialize(const std::string& filePath)
{
	//モデルマネージャーでモデルを生成
	ModelManager::GetInstance()->LoadModel(filePath);
	//モデルマネージャーから検索してセットする
	model_ = ModelManager::GetInstance()->FindModel(filePath);

	//平行光源用リソースを作る
	directionalLightResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Struct::DirectionalLight));
	//リソースにデータをセット
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//平行光源用データ
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

}

void Object3d::Draw(const WorldTransform& worldTransform, Camera* camera)
{
	//平行光源の設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, directionalLightResource->GetGPUVirtualAddress());

	//WorldTransformCBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());

	//CameraCBufferの場所特定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera->GetConstBuffer()->GetGPUVirtualAddress());

	//モデルを描画する
	model_->Draw(0, 3);
}


