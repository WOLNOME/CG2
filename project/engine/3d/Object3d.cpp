#include "Object3d.h"
#include "DirectXCommon.h"
#include "Object3dCommon.h"
#include "fstream"
#include "sstream"
#include "Function.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "WinApp.h"
#include <cassert>

void Object3d::Initialize()
{
	//平行光源用リソースを作る
	directionalLightResource = Object3dCommon::GetInstance()->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::DirectionalLight));
	//リソースにデータをセット
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//平行光源用データ
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	//モデルにカメラをセット
	model_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());
}

void Object3d::Update()
{
	model_->Update();
}

void Object3d::Draw()
{
	//平行光源の設定
	Object3dCommon::GetInstance()->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	//モデルを描画する
	model_->Draw();
}

void Object3d::SetModel(const std::string& filePath)
{
	//モデルを検索してセットする
	model_ = static_cast<std::unique_ptr<Model>>(ModelManager::GetInstance()->FindModel(filePath));
}
