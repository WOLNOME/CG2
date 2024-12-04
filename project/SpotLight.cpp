#include "SpotLight.h"
#include "DirectXCommon.h"
#include "MyMath.h"

/////--------------------------------------///
//	///			      CSMの計算
//	///--------------------------------------///
//
//	//射影空間の視錐台のコーナーを定義
//Vector3 frustumCornersNDC[8] = {
//{ -1.0f,  1.0f, 0.0f }, // ニア左上
//{  1.0f,  1.0f, 0.0f }, // ニア右上
//{  1.0f, -1.0f, 0.0f }, // ニア右下
//{ -1.0f, -1.0f, 0.0f }, // ニア左下
//{ -1.0f,  1.0f, 1.0f }, // ファー左上
//{  1.0f,  1.0f, 1.0f }, // ファー右上
//{  1.0f, -1.0f, 1.0f }, // ファー右下
//{ -1.0f, -1.0f, 1.0f }  // ファー左下
//};
////視錐台をワールド空間に変換
//Matrix4x4 inverseViewProjection = MyMath::Inverse(camera->GetViewProjectionMatrix());
//Vector3 frustumCornersWorld[8];
//for (int i = 0; i < 8; ++i) {
//	frustumCornersWorld[i] = MyMath::Transform(frustumCornersNDC[i], inverseViewProjection);
//}
////カスケードごとの分割点を計算
//std::vector<float> cascadeSplits = MyMath::CalculateCascadeSplits(kCascadeCount, camera->GetNearClip(), camera->GetFarClip());
//// カメラの nearClip と farClip を取得
//float nearClip = camera->GetNearClip();
//float farClip = camera->GetFarClip();
////各カスケードの視錐台を計算
//for (int i = 0; i < kCascadeCount; i++)
//{
//	// splitNear と splitFar
//	float splitNear = cascadeSplits[i];
//	float splitFar = cascadeSplits[i + 1];
//	//正規化
//	float norSN = (splitNear - nearClip) / (farClip - nearClip);
//	float norSF = (splitFar - nearClip) / (farClip - nearClip);
//
//
//	//AABBの宣言
//	AABB cascade;
//	// 初期値を非常に大きな値と非常に小さな値に設定
//	cascade.min = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
//	cascade.max = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
//
//	//視錐台コーナーのニアとファーを補正
//	for (int j = 0; j < 4; j++) {
//		Vector3 cornerNear = frustumCornersWorld[j] + (frustumCornersWorld[j + 4] - frustumCornersWorld[j]) * norSN;
//		Vector3 cornerFar = frustumCornersWorld[j] + (frustumCornersWorld[j + 4] - frustumCornersWorld[j]) * norSF;
//
//		// 各コーナーに基づいてAABBを更新
//		cascade.min.x = std::min({ cascade.min.x, cornerNear.x, cornerFar.x });
//		cascade.min.y = std::min({ cascade.min.y, cornerNear.y, cornerFar.y });
//		cascade.min.z = std::min({ cascade.min.z, cornerNear.z, cornerFar.z });
//
//		cascade.max.x = std::max({ cascade.max.x, cornerNear.x, cornerFar.x });
//		cascade.max.y = std::max({ cascade.max.y, cornerNear.y, cornerFar.y });
//		cascade.max.z = std::max({ cascade.max.z, cornerNear.z, cornerFar.z });
//	}
//
//	///ビュー行列を作成
//	//仮想光源位置
//	Vector3 lightPos = ((cascade.min + cascade.max) * 0.5f) - (direction_.Normalized() * Vector3(cascade.max - cascade.min).Length() * 1000000.0f);
//	//ターゲット(AABBの中心)
//	Vector3 lightTarget = (cascade.min + cascade.max) * 0.5f;
//	//仮想光源位置からディレクション方向のビュー行列
//	Vector3 up = MyMath::findOrthogonalVector(direction_.Normalized());
//	Matrix4x4 lightView = MyMath::LookAt(lightPos, lightTarget, up.Normalized());
//
//	///射影行列を作成
//	float cascadeWidth = Vector3(cascade.max - cascade.min).Length();
//	float cascadeHeight = Vector3(cascade.max - cascade.min).Length();
//	float cascadeNear = Vector3(lightTarget - lightPos).Length() - (Vector3(cascade.max - cascade.min).Length() * .0f);
//	float cascadeFar = Vector3(lightTarget - lightPos).Length() + (Vector3(cascade.max - cascade.min).Length() * 0.35f);
//	Matrix4x4 lightProjection = MyMath::MakeShadowMapProjectionMatrix(-cascadeWidth / 2.0f, cascadeHeight / 2.0f, cascadeWidth / 2.0f, -cascadeHeight / 2.0f, cascadeNear, cascadeFar);
//
//	//ライトのビュープロジェクションマトリックスを格納
//	data_.cascade[i].split = splitFar;
//	data_.cascade[i].viewProjection = lightView * lightProjection;
//}

void SpotLight::Update()
{
	//ライトの余弦をフォールオフ開始角度より大きくしないようにする処理
	if (cosFalloffStart_ < cosAngle_) {
		cosAngle_ = cosFalloffStart_ - 0.01f;
	}

	//ライト視点のviewProjectionを計算
	Vector3 up = MyMath::findOrthogonalVector(direction_.Normalized());
	Matrix4x4 lightView = MyMath::LookAt(position_, position_ + (direction_.Normalized() * distance_), up);
	float fovY = 2.0f * std::acosf(cosAngle_);
	Matrix4x4 lightProjection = MyMath::MakeShadowMapProjectionMatrix(-25.0f, 25.0f, 25.0f, -25.0f, 0.1f, 30.0f);
	//Matrix4x4 lightProjection = MyMath::MakePerspectiveFovMatrix(fovY, 1.0f, 0.1f, distance_);
	data_.viewProjection = lightView * lightProjection;

	data_.color = color_;
	data_.position = position_;
	data_.intensity = intencity_;
	data_.direction = direction_.Normalized();
	data_.distance = distance_;
	data_.decay = decay_;
	data_.cosAngle = cosAngle_;
	data_.cosFalloffStart = cosFalloffStart_;
	data_.isActive = isActive_;
}

