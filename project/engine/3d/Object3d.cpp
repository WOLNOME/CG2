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

Object3d::Object3d() {
	//リソース作成
	flagResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(FlagForPS));
	//リソースにマッピング
	flagResource_->Map(0, nullptr, reinterpret_cast<void**>(&flagData_));
	//データに書き込み
	flagData_->isActiveLights = false;
	flagData_->isActiveEnvironment = false;
}

void Object3d::InitializeModel(const std::string& filePath, ModelFormat format) {
	//モデルマネージャーでモデルを生成
	ModelManager::GetInstance()->LoadModel(filePath, format);
	//モデルマネージャーから検索してセットする
	model_ = ModelManager::GetInstance()->FindModel(filePath);

	objKind_ = kModel;
}

void Object3d::InitializeShape(Shape::ShapeKind kind) {
	//形状の生成と初期化
	shape_ = std::make_unique<Shape>();
	shape_->Initialize(kind);

	objKind_ = kShape;
}

void Object3d::Draw(WorldTransform& worldTransform, const  BaseCamera& camera, const SceneLight* sceneLight, int32_t textureHandle) {
	switch (objKind_) {
	case Object3d::kModel:
		//アニメーション反映処理
		model_->Update();

		//使用するGPSを選択
		if (model_->IsAnimation()) {
			Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::NameGPS::kAnimation);
		}
		else if (!model_->IsAnimation()) {
			Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::NameGPS::kNone);
		}

		//シーンライト有無設定
		flagData_->isActiveLights = (sceneLight != nullptr) ? true : false;

		//lightFlagCbufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(6, flagResource_->GetGPUVirtualAddress());

		//SceneLightCBufferの場所を設定
		if (flagData_->isActiveLights) {
			MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, sceneLight->GetSceneLightConstBuffer()->GetGPUVirtualAddress());
		}

		//WorldTransformCBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());

		//CameraからビュープロジェクションCBufferの場所設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());

		//Cameraからカメラ座標CBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, camera.GetCameraPositionConstBuffer()->GetGPUVirtualAddress());

		//環境光テクスチャの設定
		if (environmentLightTextureHandle_ != EOF) {
			flagData_->isActiveEnvironment = true;
			//PSにテクスチャ情報を送る
			MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(7, TextureManager::GetInstance()->GetSrvHandleGPU(environmentLightTextureHandle_));
		}
		else {
			flagData_->isActiveEnvironment = false;
		}

		//モデルを描画する
		model_->Draw(0, 3, 1, textureHandle);
		break;
	case Object3d::kShape:
		//形状の更新処理
		shape_->Update();
		//描画前設定
		if (shape_->GetShapeKind() == Shape::kSkyBox) {
			//SkyBoxの描画設定
			Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::NameGPS::kSkyBox);
		}
		else {
			//通常の描画設定
			Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::NameGPS::kNone);
		}

		//シーンライト有無設定
		flagData_->isActiveLights = (sceneLight != nullptr) ? true : false;
		//lightFlagCbufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(6, flagResource_->GetGPUVirtualAddress());
		//SceneLightCBufferの場所を設定
		if (flagData_->isActiveLights) {
			MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, sceneLight->GetSceneLightConstBuffer()->GetGPUVirtualAddress());
		}
		//WorldTransformCBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());
		//CameraからビュープロジェクションCBufferの場所設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());
		//Cameraからカメラ座標CBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, camera.GetCameraPositionConstBuffer()->GetGPUVirtualAddress());
		//環境光テクスチャの設定
		if (environmentLightTextureHandle_ != EOF) {
			flagData_->isActiveEnvironment = true;
			//PSにテクスチャ情報を送る
			MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(7, TextureManager::GetInstance()->GetSrvHandleGPU(environmentLightTextureHandle_));
		}
		else {
			flagData_->isActiveEnvironment = false;
		}

		//形状を描画する
		shape_->Draw(0, 3, 1, textureHandle);

		break;
	default:
		break;
	}
}
