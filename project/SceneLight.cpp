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
	lightViewProjectionResource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(LightViewProjectionForVS));
	//リソースをマッピング
	sceneLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&sceneLightData_));
	lightViewProjectionResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightViewProjectionData_));
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

	lightViewProjectionData_->viewProjectionMatrix = MyMath::MakeIdentity4x4();

	///------------------------------///
	///   シャドウマップ関連処理
	///------------------------------///

	//テクスチャリソース作成
	MakeTextureResource();
	//DSV設定
	SettingDSV();
	//SRV設定
	SettingSRV();

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

	lightViewProjectionData_->viewProjectionMatrix = MyMath::MakeIdentity4x4();

}

bool SceneLight::SettingGenerateShadowMap()
{
	//登録済みの光源からシャドウマップ用描画の前設定を行う
	for (; dirLightCount < directionalLights_.size();) {
		if (directionalLights_[dirLightCount]) {
			if (directionalLights_[dirLightCount]->PreDraw(selectCascadeCount,dirLightCascadeShadowTextureArray[selectCascadeCount].dsvHandle[dirLightCount])) {
				//現在のカスケードの視点情報をデータに転送
				lightViewProjectionData_->viewProjectionMatrix = directionalLights_[dirLightCount]->GetLightViewProjection(selectCascadeCount);
				//次のカスケードをとるためにカスケードカウントのインクリメント
				selectCascadeCount++;
				//生成するシャドウマップの特定ができたので、関数から抜ける
				return false;
			}
			else {
				//この光源内の全てのCSMテクスチャが手に入ったので、次の光源へ進む
				dirLightCount++;
			}
		}
	}

	//点光源やスポットライトを追加予定

	//値のリセット
	dirLightCount = 0;
	selectCascadeCount = 0;
	//全てのシャドウマップが手に入ったのでwhileループを脱出
	return true;

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

void SceneLight::MakeTextureResource()
{
	HRESULT hr;

	auto device = DirectXCommon::GetInstance()->GetDevice();

	//平行光源用カスケードシャドウテクスチャ
	for (int i = 0; i < kCascadeCount; ++i) {
		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Width = shadowMapMaxWidth_ >> i;
		textureDesc.Height = shadowMapMaxHeight_ >> i;
		textureDesc.DepthOrArraySize = kMaxNumDirectionalLight;
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&dirLightCascadeShadowTextureArray[i].textureResource)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("Failed to create directional light shadow texture.");
		}
	}

	//ポイントライト用シャドウテクスチャ(今後実装)

	//スポットライト用シャドウテクスチャ(今後実装)



}

void SceneLight::SettingDSV()
{
	HRESULT hr;

	auto device = DirectXCommon::GetInstance()->GetDevice();

	//各光源のdsvHandleのサイズを初期化
	for (int i = 0; i < kCascadeCount; ++i) {
		dirLightCascadeShadowTextureArray[i].dsvHandle.resize(kMaxNumDirectionalLight);
	}
	pointLightShadowTextureArray.dsvHandle.resize(kMaxNumPointLight);
	spotLightShadowTextureArray.dsvHandle.resize(kMaxNumSpotLight);

	//DSVデスクリプタヒープを作成
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = (kMaxNumDirectionalLight * kCascadeCount) + kMaxNumPointLight + kMaxNumSpotLight;//(平行光源数*カスケード分割数)+点光源数+スポットライト数
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap_));

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to make dsv descriptor heap.");
	}

	///平行光源用シャドウテクスチャ

	//DSVを作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescDirLight_ = {};
	dsvDescDirLight_.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDescDirLight_.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDescDirLight_.Flags = D3D12_DSV_FLAG_NONE;
	dsvDescDirLight_.Texture2DArray.ArraySize = kMaxNumDirectionalLight;

	// DSV ヒープにシャドウマップをバインド
	for (int i = 0; i < kMaxNumDirectionalLight; ++i) {
		for (int j = 0; j < kCascadeCount; ++j) {
			//各平行光源ごとのカスケードに対応するDSVハンドルを計算
			dirLightCascadeShadowTextureArray[j].dsvHandle[i] =
				CD3DX12_CPU_DESCRIPTOR_HANDLE(
					dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(),
					i * kCascadeCount + j,		//デスクリプタヒープの場所
					device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
			);
			// FirstArraySlice を変更して、個別にスライスを指定
			dsvDescDirLight_.Texture2DArray.FirstArraySlice = j;

			//DepthStencilViewを作成してバインド
			device->CreateDepthStencilView(
				dirLightCascadeShadowTextureArray[j].textureResource.Get(),
				&dsvDescDirLight_,
				dirLightCascadeShadowTextureArray[j].dsvHandle[i]
			);

		}
	}


	//ポイントライト用シャドウテクスチャ(今後実装)

	//スポットライト用シャドウテクスチャ(今後実装)


}

void SceneLight::SettingSRV()
{
	// 平行光源カスケードシャドウマップのSRVを登録
	for (int i = 0; i < kCascadeCount; ++i) {
		// 空きインデックスを確保
		dirLightCascadeShadowTextureArray[i].srvIndex = SrvManager::GetInstance()->Allocate();

		// SRVを生成
		SrvManager::GetInstance()->CreateSRVforTexture2D(
			dirLightCascadeShadowTextureArray[i].srvIndex,
			dirLightCascadeShadowTextureArray[i].textureResource.Get(),
			DXGI_FORMAT_R32_FLOAT, // シャドウマップは通常R32_FLOATで扱います
			1 // ミップマップレベル
		);

	}


	//ポイントライト用シャドウテクスチャ(今後実装)

	//スポットライト用シャドウテクスチャ(今後実装)

}
