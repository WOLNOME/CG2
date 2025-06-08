#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <array>
#include <string>
#include "Vector4.h"
#include "Vector3.h"

// データ構造体(サイズが16の倍数になるようにパディングする！)
struct DirectionalLightData {
	Vector4 color;
	Vector3 direction;
	float intensity;
	uint32_t isActive;
	float padding[3];
};

/// <summary>
/// 平行光源
/// </summary>
class DirectionalLight {
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
	void Update();
	/// <summary>
	/// 平行光源のデータを取得
	/// </summary>
	/// <returns>平行光源のデータ</returns>
	const DirectionalLightData& GetData() const { return data_; }
	/// <summary>
	/// デバッグ用ImGui
	/// </summary>
	void DebugWithImGui(const std::wstring& _name);

private:
	//データ
	DirectionalLightData data_;

	// コピー禁止
	DirectionalLight(const DirectionalLight&) = delete;
	DirectionalLight& operator=(const DirectionalLight&) = delete;
};

static_assert(!std::is_copy_assignable_v<DirectionalLight>);
