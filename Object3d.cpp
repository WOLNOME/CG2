#include "Object3d.h"
#include "DirectXCommon.h"
#include "Object3dCommon.h"
#include "fstream"
#include "sstream"
#include "Function.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "Model.h"
#include <cassert>

void Object3d::Initialize(Object3dCommon* modelCommon)
{
	//引数で受け取ってメンバ変数に記録する
	object3dCommon_ = modelCommon;

	//平行光源用リソースを作る
	directionalLightResource = object3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::DirectionalLight));
	//リソースにデータをセット
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//平行光源用データ
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

}

void Object3d::Update()
{
	for (Model* model : models_) {
		model->Update();
	}
}

void Object3d::Draw()
{
	//平行光源の設定
	object3dCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	//モデルを描画する
	for (Model* model : models_) {
		model->Draw();
	}
}
