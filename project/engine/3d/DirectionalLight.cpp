#include "DirectionalLight.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "MyMath.h"
#undef min
#undef max
#include <algorithm>
#include <numbers>

void DirectionalLight::Initialize()
{
}

void DirectionalLight::Update(BaseCamera* camera)
{
	///--------------------------------------///
	///			      CSMの計算
	///--------------------------------------///

	//射影空間の視錐台のコーナーを定義
	Vector3 frustumCornersNDC[8] = {
	{ -1.0f,  1.0f, 0.0f }, // ニア左上
	{  1.0f,  1.0f, 0.0f }, // ニア右上
	{  1.0f, -1.0f, 0.0f }, // ニア右下
	{ -1.0f, -1.0f, 0.0f }, // ニア左下
	{ -1.0f,  1.0f, 1.0f }, // ファー左上
	{  1.0f,  1.0f, 1.0f }, // ファー右上
	{  1.0f, -1.0f, 1.0f }, // ファー右下
	{ -1.0f, -1.0f, 1.0f }  // ファー左下
	};
	//視錐台をワールド空間に変換
	Matrix4x4 inverseViewProjection = MyMath::Inverse(camera->GetViewProjectionMatrix());
	Vector3 frustumCornersWorld[8];
	for (int i = 0; i < 8; ++i) {
		frustumCornersWorld[i] = MyMath::Transform(frustumCornersNDC[i], inverseViewProjection);
	}
	//カスケードごとの分割点を計算
	std::vector<float> cascadeSplits = MyMath::CalculateCascadeSplits(kCascadeCount, camera->GetNearClip(), camera->GetFarClip());
	// カメラの nearClip と farClip を取得
	float nearClip = camera->GetNearClip();
	float farClip = camera->GetFarClip();
	//各カスケードの視錐台を計算
	for (int i = 0; i < kCascadeCount; i++)
	{
		// splitNear と splitFar
		float splitNear = cascadeSplits[i];
		float splitFar = cascadeSplits[i + 1];
		//正規化
		float norSN = (splitNear - nearClip) / (farClip - nearClip);
		float norSF = (splitFar - nearClip) / (farClip - nearClip);


		//AABBの宣言
		AABB cascade;
		// 初期値を非常に大きな値と非常に小さな値に設定
		cascade.min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		cascade.max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		//視錐台コーナーのニアとファーを補正
		for (int j = 0; j < 4; j++) {
			Vector3 cornerNear = frustumCornersWorld[j] + (frustumCornersWorld[j + 4] - frustumCornersWorld[j]) * norSN;
			Vector3 cornerFar = frustumCornersWorld[j] + (frustumCornersWorld[j + 4] - frustumCornersWorld[j]) * norSF;

			// 各コーナーに基づいてAABBを更新
			cascade.min.x = std::min({ cascade.min.x, cornerNear.x, cornerFar.x });
			cascade.min.y = std::min({ cascade.min.y, cornerNear.y, cornerFar.y });
			cascade.min.z = std::min({ cascade.min.z, cornerNear.z, cornerFar.z });

			cascade.max.x = std::max({ cascade.max.x, cornerNear.x, cornerFar.x });
			cascade.max.y = std::max({ cascade.max.y, cornerNear.y, cornerFar.y });
			cascade.max.z = std::max({ cascade.max.z, cornerNear.z, cornerFar.z });
		}

		///ビュー行列を作成
		//仮想光源位置
		Vector3 lightPos = (cascade.min + cascade.max) * 0.5f - (direction_.Normalized() * Vector3(cascade.max - cascade.min).Length() * 0.5f);
		//ターゲット(AABBの中心)
		Vector3 lightTarget = (cascade.min + cascade.max) * 0.5f;
		//仮想光源位置からディレクション方向のビュー行列
		Vector3 up = MyMath::findOrthogonalVector(direction_.Normalized());
		Matrix4x4 lightView = MyMath::LookAt(lightPos, lightTarget, up.Normalized());
		
		///射影行列を作成
		float cascadeWidth = Vector3(cascade.max - cascade.min).Length();
		float cascadeHeight = Vector3(cascade.max - cascade.min).Length();
		float cascadeDepth = Vector3(lightTarget - lightPos).Length() + (Vector3(cascade.max - cascade.min).Length() * 0.5f);
		Matrix4x4 lightProjection = MyMath::MakeShadowMapProjectionMatrix(-cascadeWidth / 2.0f, cascadeHeight / 2.0f, cascadeWidth / 2.0f, -cascadeHeight / 2.0f, 0.1f, cascadeDepth);
		
		//ライトのビュープロジェクションマトリックスを格納
		data_.cascade[i].split = splitFar;
		data_.cascade[i].viewProjection = lightView * lightProjection;
	}

	//データを転送
	data_.color = color_;
	data_.direction = direction_;
	data_.intensity = intencity_;
	data_.numCascade = kCascadeCount;
	data_.isActive = isActive_;
}
