#pragma once
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include <d3d12.h>
#include <wrl.h>
#include <vector>
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

class SceneLight
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 定数バッファの取得
	/// </summary>
	/// <returns>定数バッファ</returns>
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetConstBuffer() const { return resource_; }

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
	//定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	//マッピング済みアドレス
	SceneLightForPS* data_ = nullptr;

	//シーン内にある平行光源
	std::vector<DirectionalLight*> directionalLights_;
	//シーン内にある点光源
	std::vector<PointLight*> pointLights_;
	//シーン内にあるスポットライト
	std::vector<SpotLight*> spotLights_;


};

