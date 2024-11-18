#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Vector4.h"
#include "Vector3.h"


// 定数バッファ用データ構造体
struct DirectionalLightForPS {
	Vector4 color;
	Vector3 direction;
	float intensity;
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
	void Update();
	/// <summary>
	/// 定数バッファの取得
	/// </summary>
	/// <returns>定数バッファ</returns>
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetConstBuffer() const { return resource_; }

private:
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	// マッピング済みアドレス
	DirectionalLightForPS* data_ = nullptr;
	// コピー禁止
	DirectionalLight(const DirectionalLight&) = delete;
	DirectionalLight& operator=(const DirectionalLight&) = delete;
};

static_assert(!std::is_copy_assignable_v<DirectionalLight>);
