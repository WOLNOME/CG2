#include "DirectionalLight.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "MyMath.h"
#include <algorithm>

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

	//各カスケードの視錐台を計算
	for (int i = 0; i < kCascadeCount; i++)
	{
		float splitNear = cascadeSplits[i];
		float splitFar = cascadeSplits[i + 1];

		//AABBの宣言
		AABB cascade;

		//視錐台コーナーのニアとファーを補正
		for (int j = 0; j < 4; j++) {
			Vector3 cornerNear = frustumCornersWorld[j] + (frustumCornersWorld[j + 4] - frustumCornersWorld[j]) * splitNear;
			Vector3 cornerFar = frustumCornersWorld[j] + (frustumCornersWorld[j + 4] - frustumCornersWorld[j]) * splitFar;

			//AABBの最小値・最大値を計算
			cascade.min.x = min(cornerNear.x, cornerFar.x);
			cascade.min.y = min(cornerNear.y, cornerFar.y);
			cascade.min.z = min(cornerNear.z, cornerFar.z);

			cascade.max.x = max(cornerNear.x, cornerFar.x);
			cascade.max.y = max(cornerNear.y, cornerFar.y);
			cascade.max.z = max(cornerNear.z, cornerFar.z);
		}

		///ビュー行列を作成

		//仮想光源位置
		Vector3 lightPos = (cascade.min + cascade.max) * 0.5f - direction_.Normalized() * (cascade.max.z - cascade.min.z);
		//ターゲット(AABBの中心)
		Vector3 lightTarget = (cascade.min + cascade.max) * 0.5f;
		//仮想光源位置からディレクション方向のビュー行列
		Matrix4x4 lightView = MyMath::LookAt(lightPos, lightTarget, Vector3(0.0f, 1.0f, 0.0f));

		///射影行列を作成

		float cascadeWidth = cascade.max.x - cascade.min.x;
		float cascadeHeight = cascade.max.y - cascade.min.y;
		float cascadeDepth = cascade.max.z - cascade.min.z;
		Matrix4x4 lightProjection = MyMath::MakeOrthographicMatrix(-cascadeWidth / 2, cascadeHeight / 2, cascadeWidth / 2, -cascadeHeight / 2, 0.0f, cascadeDepth);

		//保存用ビュープロジェクションに登録
		lightViewProjection[i] = lightView * lightProjection;

		//ライトのビュープロジェクションマトリックスを格納
		data_.cascade.lightViewProjectionMatrix[i] = lightViewProjection[i];
		data_.cascade.cascadeSplits[i] = splitFar;// シェーダ用にファークリップを保存
	}

	//データを転送
	data_.color = color_;
	data_.direction = direction_;
	data_.intensity = intencity_;
	data_.isActive = isActive_;
}

bool DirectionalLight::PreDraw(int cascadeNum, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
{
	//送られてきた番号のカスケードが存在しなかったら失敗判定
	if (cascadeNum >= kCascadeCount) {
		return false;
	}

	auto commandList = DirectXCommon::GetInstance()->GetCommandList();

	// DSVを設定
	commandList->OMSetRenderTargets(0, nullptr, false, &dsvHandle);

	// 深度ステンシルバッファをクリア
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	return true;
}

const Matrix4x4& DirectionalLight::GetLightViewProjection(int cascadeNum)
{
	return lightViewProjection[cascadeNum];
}
