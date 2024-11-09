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
	//モデルにカメラをセット
	model_->SetCamera(Object3dCommon::GetInstance()->GetDefaultCamera());

	//Object3dのリソースを作成
	object3dResource_=MakeObject3dResource();

	//平行光源用リソースを作る
	directionalLightResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Struct::DirectionalLight));
	//リソースにデータをセット
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//平行光源用データ
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

}

void Object3d::Update()
{
	//レンダリングパイプライン
	Matrix4x4 worldMatrix = MyMath::MakeAffineMatrix(object3dResource_.transform.scale, object3dResource_.transform.rotate, object3dResource_.transform.translate);
	Matrix4x4 worldViewProjectionMatrix;
	if (model_->GetCamera()) {
		const Matrix4x4& viewProjectionMatrix = model_->GetCamera()->GetViewProjectionMatrix();
		worldViewProjectionMatrix = MyMath::Multiply(worldMatrix, viewProjectionMatrix);
	}
	else {
		//一応カメラが無くても処理は通るが正しく表示はされない
		worldViewProjectionMatrix = worldMatrix;
	}
	object3dResource_.wvpData->WVP = worldViewProjectionMatrix;
	object3dResource_.wvpData->World = worldMatrix;
	
	//見た目の更新
	model_->Update();
}

void Object3d::Draw()
{
	//平行光源の設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	//座標変換行列CBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, object3dResource_.wvpResource->GetGPUVirtualAddress());

	//モデルを描画する
	model_->Draw();
}

Object3d::Struct::Object3dResource Object3d::MakeObject3dResource()
{
	//Object3dリソース
	Struct::Object3dResource object3dResource_;
	//WVPリソース作成
	object3dResource_.wvpResource= DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Struct::TransformationMatrix));
	//リソースにデータを書き込む
	object3dResource_.wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&object3dResource_.wvpData));
	//データに書き込む
	object3dResource_.wvpData->WVP = MyMath::MakeIdentity4x4();
	object3dResource_.wvpData->World = MyMath::MakeIdentity4x4();
	//トランスフォーム
	object3dResource_.transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};
	//リターン
	return object3dResource_;
}
