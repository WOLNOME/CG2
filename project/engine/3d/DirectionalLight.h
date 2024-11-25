#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl.h>
#include "Vector4.h"
#include "Vector3.h"
#include "BaseCamera.h"

//カスケードの分割数
const int kCascadeCount = 3;
// シャドウマップの幅と高さ
constexpr UINT shadowMapMaxWidth_ = 2048;
constexpr UINT shadowMapMaxHeight_ = 2048;

struct CascadeData {
	Matrix4x4 lightViewProjectionMatrix[kCascadeCount];
	float cascadeSplits[kCascadeCount];
};
// データ構造体(サイズが16の倍数になるようにパディングする！)
struct DirectionalLightData {
	Vector4 color;
	Vector3 direction;
	float intensity;
	CascadeData cascade;
	uint32_t isActive;
};

/// <summary>
/// 平行光源
/// </summary>
class DirectionalLight
{
public:
	//色
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	//向き
	Vector3 direction_ = { 0.0f, -1.0f, 0.0f };
	//輝度
	float intencity_ = 1.0f;
	//オンオフ
	bool isActive_ = true;

	DirectionalLight() = default;
	~DirectionalLight() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 行列の更新
	/// </summary>
	void Update(BaseCamera* camera);
	/// <summary>
	/// カスケードごとの描画前設定
	/// </summary>
	/// <param name="cascadeNum">カスケードの番号(最初は0番)</param>
	/// <returns>設定の成功判定</returns>
	bool PreDraw(int cascadeNum, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);
	/// <summary>
	/// 平行光源のデータを取得
	/// </summary>
	/// <returns>スポットライトのデータ</returns>
	const DirectionalLightData& GetData() { return data_; }
	/// <summary>
	/// ライト目線のViewProjection
	/// </summary>
	/// <param name="cascadeNum">カスケードの番号(最初は0番)</param>
	const Matrix4x4& GetLightViewProjection(int cascadeNum);

private:
	//データ
	DirectionalLightData data_;

	//ライトのビュープロジェクション(保存用)
	Matrix4x4 lightViewProjection[kCascadeCount];

	// コピー禁止
	DirectionalLight(const DirectionalLight&) = delete;
	DirectionalLight& operator=(const DirectionalLight&) = delete;
};

static_assert(!std::is_copy_assignable_v<DirectionalLight>);
