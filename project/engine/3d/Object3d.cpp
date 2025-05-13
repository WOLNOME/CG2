#include "Object3d.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "Object3dCommon.h"
#include "TextureManager.h"
#include "SrvManager.h"
#include "ModelManager.h"
#include "SceneLight.h"
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
	//ワールドトランスフォームの初期化
	worldTransform.Initialize();
}

void Object3d::Initialize(ModelTag, const std::string& filePath, ModelFormat format) {
	//モデルマネージャーでモデルを生成
	ModelManager::GetInstance()->LoadModel(filePath, format);
	//モデルマネージャーから検索してセットする
	model_ = ModelManager::GetInstance()->FindModel(filePath);

	objKind_ = ObjectKind::Model;
}

void Object3d::Initialize(AnimationModelTag, const std::string& filePath, ModelFormat format) {
	//モデルマネージャーでアニメーションモデルを生成
	ModelManager::GetInstance()->LoadAnimationModel(filePath, format);
	//モデルマネージャーから検索してセットする
	animationModel_ = ModelManager::GetInstance()->FindAnimationModel(filePath);

	objKind_ = ObjectKind::AnimationModel;
}

void Object3d::Initialize(ShapeTag, Shape::ShapeKind kind) {
	//形状の生成と初期化
	shape_ = std::make_unique<Shape>();
	shape_->Initialize(kind);

	objKind_ = ObjectKind::Shape;
}

void Object3d::Update() {
	//ワールドトランスフォームの更新
	worldTransform.UpdateMatrix();
}

void Object3d::Draw(const BaseCamera* _camera, int32_t _textureHandle) {
	switch (objKind_) {
	case ObjectKind::Model:
		//通常モデル用共通描画の設定
		Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::NameGPS::None);
		//シーンライト有無設定
		flagData_->isActiveLights = (light) ? true : false;

		//lightFlagCbufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(6, flagResource_->GetGPUVirtualAddress());

		//SceneLightCBufferの場所を設定
		if (flagData_->isActiveLights) {
			MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, light->GetSceneLightConstBuffer()->GetGPUVirtualAddress());
		}

		//WorldTransformCBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());

		//CameraからビュープロジェクションCBufferの場所設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, _camera->GetViewProjectionConstBuffer()->GetGPUVirtualAddress());

		//Cameraからカメラ座標CBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, _camera->GetCameraPositionConstBuffer()->GetGPUVirtualAddress());

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
		model_->Draw(0, 3, 1, _textureHandle);
		break;
	case ObjectKind::AnimationModel:
		//アニメーション反映処理
		animationModel_->Update();

		//アニメーションモデル用共通描画の設定
		Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::NameGPS::Animation);

		//シーンライト有無設定
		flagData_->isActiveLights = (light) ? true : false;

		//lightFlagCbufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(6, flagResource_->GetGPUVirtualAddress());

		//SceneLightCBufferの場所を設定
		if (flagData_->isActiveLights) {
			MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, light->GetSceneLightConstBuffer()->GetGPUVirtualAddress());
		}

		//WorldTransformCBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());

		//CameraからビュープロジェクションCBufferの場所設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, _camera->GetViewProjectionConstBuffer()->GetGPUVirtualAddress());

		//Cameraからカメラ座標CBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, _camera->GetCameraPositionConstBuffer()->GetGPUVirtualAddress());

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
		animationModel_->Draw(0, 3, 1, _textureHandle);
		break;
	case ObjectKind::Shape:
		//形状の更新処理
		shape_->Update();
		//描画前設定
		if (shape_->GetShapeKind() == Shape::kSkyBox) {
			//SkyBoxの描画設定
			Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::NameGPS::SkyBox);
		}
		else {
			//通常の描画設定
			Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::NameGPS::None);
		}

		//シーンライト有無設定
		flagData_->isActiveLights = (light) ? true : false;
		//lightFlagCbufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(6, flagResource_->GetGPUVirtualAddress());
		//SceneLightCBufferの場所を設定
		if (flagData_->isActiveLights) {
			MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, light->GetSceneLightConstBuffer()->GetGPUVirtualAddress());
		}
		//WorldTransformCBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());
		//CameraからビュープロジェクションCBufferの場所設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, _camera->GetViewProjectionConstBuffer()->GetGPUVirtualAddress());
		//Cameraからカメラ座標CBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, _camera->GetCameraPositionConstBuffer()->GetGPUVirtualAddress());
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
		shape_->Draw(0, 3, 1, _textureHandle);

		break;
	default:
		break;
	}
}
