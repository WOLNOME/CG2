#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Matrix4x4.h"
#include "Vector4.h"
#include "Vector3.h"

//

// データ構造体(サイズが16の倍数になるようにパディングする！)
struct SpotLightData {
	Vector4 color;				//ライトの色
	Vector3 position;			//ライトの位置
	float intensity;			//輝度
	Vector3 direction;			//スポットライトの方向
	float distance;				//ライトの届く最大距離
	float decay;				//減衰率
	float cosAngle;				//スポットライトの余弦
	float cosFalloffStart;		//フォールオフの開始角度
	uint32_t isActive;
};
/// <summary>
/// スポットライト
/// </summary>
class SpotLight
{
public:
	//色
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	//向き
	Vector3 position_ = { 0.0f, 0.0f, 0.0f };
	//輝度
	float intencity_ = 1.0f;
	//ライトの方向
	Vector3 direction_ = { -1.0f,-1.0f,0.0f };
	//光の有効距離
	float distance_ = 5.0f;
	//減衰率
	float decay_ = 0.5f;
	//ライトの余弦
	float cosAngle_ = 0.7f;
	//フォールオフ開始角度
	float cosFalloffStart_ = 1.0f;
	//オンオフ
	bool isActive_ = true;

	SpotLight() = default;
	~SpotLight() = default;

	/// <summary>
	/// 行列の更新
	/// </summary>
	void Update();
	/// <summary>
	/// スポットライトのデータを取得
	/// </summary>
	/// <returns>スポットライトのデータ</returns>
	const SpotLightData& GetData() { return data_; }

private:
	//データ
	SpotLightData data_;

	// コピー禁止
	SpotLight(const SpotLight&) = delete;
	SpotLight& operator=(const SpotLight&) = delete;
};

static_assert(!std::is_copy_assignable_v<SpotLight>);
