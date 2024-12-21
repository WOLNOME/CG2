#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Vector4.h"
#include "Vector3.h"


// データ構造体(サイズが16の倍数になるようにパディングする！)
struct PointLightData {
	Vector4 color;		//ライトの色
	Vector3 position;	//ライトの位置
	float intensity;	//輝度
	float radius;		//ライトの届く最大距離
	float decay;		//減衰率
	uint32_t isActive;
	float padding[1];
};
/// <summary>
/// 点光源
/// </summary>
class PointLight
{
public:
	//色
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	//向き
	Vector3 position_ = { 0.0f, 0.0f, 0.0f };
	//輝度
	float intencity_ = 1.0f;
	//光の有効距離
	float radius_ = 5.0f;
	//減衰率
	float decay_ = 0.5f;
	//オンオフ
	bool isActive_ = true;

	PointLight() = default;
	~PointLight() = default;

	/// <summary>
	/// 行列の更新
	/// </summary>
	void Update();
	/// <summary>
	/// 平行光源のデータを取得
	/// </summary>
	/// <returns>スポットライトのデータ</returns>
	const PointLightData& GetData() { return data_; }
	
private:
	// データ
	PointLightData data_;
	// コピー禁止
	PointLight(const PointLight&) = delete;
	PointLight& operator=(const PointLight&) = delete;
};

static_assert(!std::is_copy_assignable_v<PointLight>);
