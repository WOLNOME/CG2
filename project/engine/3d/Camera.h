#pragma once
#include <d3d12.h>
#include <type_traits>
#include <wrl.h>
#include "MyMath.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

// 定数バッファ用データ構造体
struct ViewProjectionTransformationMatrixForVS {
	Matrix4x4 matView;		 // ワールド → ビュー変換行列
	Matrix4x4 matProjection; // ビュー → プロジェクション変換行列
	Vector4 worldCameraPos;		 // カメラ座標（ワールド座標）
};

//カメラ
class Camera
{
public:
	Camera();

	//初期化
	void Initialize();
	//更新
	void UpdateMatrix();

public://ゲッター
	const Matrix4x4& GetWorldMatrix()const { return worldMatrix; }
	const Matrix4x4& GetViewMatrix()const { return viewMatrix; }
	const Matrix4x4& GetProjectionMatrix()const { return projectionMatrix; }
	const Matrix4x4& GetViewProjectionMatrix()const { return viewProjectionMatrix; }
	const Vector3& GetRotate()const { return transform.rotate; }
	const Vector3& GetTranslate()const { return transform.translate; }
	/// <summary>
	/// 定数バッファの取得
	/// </summary>
	/// <returns>定数バッファ</returns>
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetConstBuffer() const { return resource_; }

public://セッター
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	void SetFovY(float fovY) { this->fovY = fovY; }
	void SetAspectRatio(float aspectRatio) { this->aspectRatio = aspectRatio; }
	void SetNearClip(float  nearClip) { this->nearClip = nearClip; }
	void SetFarClip(float farClip) { this->farClip = farClip; }


private:
	// 定数バッファ(座標変換リソース)
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
	// マッピング済みアドレス
	ViewProjectionTransformationMatrixForVS* data_ = nullptr;
	// コピー禁止
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;

	Transform transform;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 viewProjectionMatrix;

	float fovY;//水平方向視野角
	float aspectRatio;//アスペクト比
	float nearClip;//ニアクリップ距離
	float farClip;//ファークリップ距離
};

static_assert(!std::is_copy_assignable_v<Camera>);
