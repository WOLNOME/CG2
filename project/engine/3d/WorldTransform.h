#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "Matrix4x4.h"
#include "Vector3.h"
#include "MyMath.h"

// 定数バッファ用データ構造体
struct WorldTransformationMatrixForVS {
	Matrix4x4 matWorld; // ローカル → ワールド変換行列
	Matrix4x4 matWorldInverseTranspose; //ワールド逆転置行列
};

/// <summary>
/// ワールド変換データ
/// </summary>
class WorldTransform
{
public:
	// ローカルのスケール
	Vector3 scale_ = { 1, 1, 1 };
	// ローカルの回転角
	Vector3 rotate_ = { 0, 0, 0 };
	// ローカルの座標
	Vector3 translate_ = { 0, 0, 0 };
	// ローカル → ワールド変換行列
	Matrix4x4 matWorld_;
	// 親となるワールド変換へのポインタ
	const WorldTransform* parent_ = nullptr;

	WorldTransform() = default;
	~WorldTransform() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// データの更新
	/// </summary>
	void UpdateMatrix(const Matrix4x4& local = MyMath::MakeIdentity4x4());
	/// <summary>
	/// 定数バッファの取得
	/// </summary>
	/// <returns>定数バッファ</returns>
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetConstBuffer() const { return resource_; }

private:
	// 定数バッファ(座標変換リソース)
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	// マッピング済みアドレス
	WorldTransformationMatrixForVS* data_ = nullptr;
	// コピー禁止
	WorldTransform(const WorldTransform&) = delete;
	WorldTransform& operator=(const WorldTransform&) = delete;

};

static_assert(!std::is_copy_assignable_v<WorldTransform>);