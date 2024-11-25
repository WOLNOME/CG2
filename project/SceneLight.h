#pragma once
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "BaseCamera.h"
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <array>
#include <memory>

//点光源の最大数
const uint32_t kMaxNumDirectionalLight = 1;
const uint32_t kMaxNumPointLight = 16;
const uint32_t kMaxNumSpotLight = 16;

//定数バッファ用データ構造体
struct SceneLightForPS
{
	DirectionalLightData directionalLights[kMaxNumDirectionalLight];
	PointLightData pointLights[kMaxNumPointLight];
	SpotLightData spotLights[kMaxNumSpotLight];
	uint32_t numDirectionalLights;
	uint32_t numPointLights;
	uint32_t numSpotLights;
};
struct LightViewProjectionForVS
{
	Matrix4x4 viewProjectionMatrix;
};


class SceneLight
{
public:
	//シャドウマップ情報
	struct ShadowTexture
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> dsvHandle;			//光源の数(Texture2Dのスライス数)だけ必要サイズは固定できないのでvector型
		uint32_t srvIndex;											//Texture2D1つにつき1個でいいのでvectorにはしない
	};

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update(BaseCamera* camera);
	/// <summary>
	/// シャドウマップ生成用設定
	/// </summary>
	/// <returns>全てのシャドウマップを生成済み判定</returns>
	bool SettingGenerateShadowMap();
	/// <summary>
	/// シーンライト用定数バッファの取得
	/// </summary>
	/// <returns>シーンライト用定数バッファ</returns>
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetSceneLightConstBuffer() const { return sceneLightResource_; }
	/// <summary>
	/// ビュープロジェクション用定数バッファの取得
	/// </summary>
	/// <returns>ビュープロジェクション用定数バッファ</returns>
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetLightViewProjectionConstBuffer() const { return lightViewProjectionResource_; }

	/// <summary>
	/// 平行光源をセット
	/// </summary>
	/// <param name="dirLight">平行光源のポインタ</param>
	void SetLight(DirectionalLight* dirLight);
	/// <summary>
	/// 点光源をセット
	/// </summary>
	/// <param name="pointLight">点光源のポインタ</param>
	void SetLight(PointLight* pointLight);
	/// <summary>
	/// スポットライトをセット
	/// </summary>
	/// <param name="spotLight">スポットライトのポインタ</param>
	void SetLight(SpotLight* spotLight);

private:
	//テクスチャリソース作成用関数
	void MakeTextureResource();
	//DSV設定
	void SettingDSV();
	//SRV設定
	void SettingSRV();

private:
	//シーンライト用定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> sceneLightResource_;
	//シーンライト用マッピング済みアドレス
	SceneLightForPS* sceneLightData_ = nullptr;
	//シャドウマップに送る用の各ライトの視点用定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> lightViewProjectionResource_;
	//シャドウマップに送る用の各ライトの視点用マッピング済みアドレス
	LightViewProjectionForVS* lightViewProjectionData_ = nullptr;

	//シーン内にある平行光源
	std::vector<DirectionalLight*> directionalLights_;
	//シーン内にある点光源
	std::vector<PointLight*> pointLights_;
	//シーン内にあるスポットライト
	std::vector<SpotLight*> spotLights_;

	//シーン内にある平行光源のカウント
	int dirLightCount = 0;
	//１つの光源内のカスケードカウント
	int selectCascadeCount = 0;



	///------------------------------///
	///     シャドウマップ関連
	///------------------------------///

	//シャドウマップ用DSVのデスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;


	//テクスチャリソース
	std::array<ShadowTexture, kCascadeCount> dirLightCascadeShadowTextureArray;
	ShadowTexture pointLightShadowTextureArray;
	ShadowTexture spotLightShadowTextureArray;

};

