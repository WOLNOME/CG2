#include "SceneLight.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <cassert>
#include <algorithm>

void SceneLight::Initialize()
{
	//各光源のサイズを確定
	directionalLights_.resize(kMaxNumDirectionalLight);
	pointLights_.resize(kMaxNumPointLight);
	spotLights_.resize(kMaxNumSpotLight);

	//リソースの作成
	sceneLightResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(SceneLightForPS));
	//リソースをマッピング
	sceneLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&sceneLightData_));
	//データを書き込む
	int index = 0;
	for (const auto& dirLight : directionalLights_) {
		if (dirLight) {
			sceneLightData_->directionalLights[index] = dirLight->GetData();
			index++;
		}
	}
	sceneLightData_->numDirectionalLights = index;
	index = 0;
	for (const auto& pointLight : pointLights_) {
		if (pointLight) {
			sceneLightData_->pointLights[index] = pointLight->GetData();
			index++;
		}
	}
	sceneLightData_->numPointLights = index;
	index = 0;
	for (const auto& spotLight : spotLights_) {
		if (spotLight) {
			sceneLightData_->spotLights[index] = spotLight->GetData();
			index++;
		}
	}
	sceneLightData_->numSpotLights = index;

}

void SceneLight::Update(BaseCamera* camera)
{
	//登録済みの光源の更新処理
	for (const auto& dirLight : directionalLights_) {
		if (dirLight) {
			dirLight->Update(camera);
		}
	}
	for (const auto& pointLight : pointLights_) {
		if (pointLight) {
			pointLight->Update();
		}
	}
	for (const auto& spotLight : spotLights_) {
		if (spotLight) {
			spotLight->Update();
		}
	}

	//データの更新
	int index = 0;
	for (const auto& dirLight : directionalLights_) {
		if (dirLight) {
			sceneLightData_->directionalLights[index] = dirLight->GetData();
			index++;
		}
	}
	sceneLightData_->numDirectionalLights = index;
	index = 0;
	for (const auto& pointLight : pointLights_) {
		if (pointLight) {
			sceneLightData_->pointLights[index] = pointLight->GetData();
			index++;
		}
	}
	sceneLightData_->numPointLights = index;
	index = 0;
	for (const auto& spotLight : spotLights_) {
		if (spotLight) {
			sceneLightData_->spotLights[index] = spotLight->GetData();
			index++;
		}
	}
	sceneLightData_->numSpotLights = index;

}

void SceneLight::SetLight(DirectionalLight* dirLight)
{
	// 現在登録されている光源の数を取得
	size_t currentNumLights = std::count_if(
		directionalLights_.begin(),
		directionalLights_.end(),
		[](DirectionalLight* light) { return light != nullptr; }
	);

	// 最大値を超える場合はassertでエラーを吐く
	assert(currentNumLights < kMaxNumDirectionalLight && "Too many directional lights!");

	// 空いているスロットに登録
	for (auto& light : directionalLights_) {
		if (light == nullptr) {
			light = dirLight;
			return;
		}
	}

	// ここに来ることは通常ありえない（resizeで確保しているため）
	assert(false && "Failed to set light - no available slots!");
}

void SceneLight::SetLight(PointLight* pointLight)
{
	// 現在登録されている光源の数を取得
	size_t currentNumLights = std::count_if(
		pointLights_.begin(),
		pointLights_.end(),
		[](PointLight* light) { return light != nullptr; }
	);

	// 最大値を超える場合はassertでエラーを吐く
	assert(currentNumLights < kMaxNumPointLight && "Too many point lights!");

	// 空いているスロットに登録
	for (auto& light : pointLights_) {
		if (light == nullptr) {
			light = pointLight;
			return;
		}
	}

	// ここに来ることは通常ありえない（resizeで確保しているため）
	assert(false && "Failed to set light - no available slots!");
}

void SceneLight::SetLight(SpotLight* spotLight)
{
	// 現在登録されている光源の数を取得
	size_t currentNumLights = std::count_if(
		spotLights_.begin(),
		spotLights_.end(),
		[](SpotLight* light) { return light != nullptr; }
	);

	// 最大値を超える場合はassertでエラーを吐く
	assert(currentNumLights < kMaxNumSpotLight && "Too many spot lights!");

	// 空いているスロットに登録
	for (auto& light : spotLights_) {
		if (light == nullptr) {
			light = spotLight;
			return;
		}
	}

	// ここに来ることは通常ありえない（resizeで確保しているため）
	assert(false && "Failed to set light - no available slots!");
}
