#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include"Function.h"

class Sprite
{
public://インナークラス
	class Struct {
	public:
		//頂点データ
		struct VertexData {
			Vector4 position;
			Vector2 texcoord;
			Vector3 normal;
		};
		//平行光源データ
		struct DirectionalLight
		{
			Vector4 color;
			Vector3 direction;
			float intensity;
		};
		//マテリアルデータ
		struct Material {
			Vector4 color;
			int32_t lightingKind;
			float padding[3];
			Matrix4x4 uvTransform;
			int32_t isTexture;
		};
		//座標変換行列データ
		struct TransformationMatrix {
			Matrix4x4 WVP;
			Matrix4x4 World;
		};

	};

public://メンバ関数
	void Initialize(const std::string& textureFilePath);
	void Update();
	void Draw();
private://メンバ関数
	//テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

public://ゲッター
	const Vector2& GetPosition()const { return position; }
	float GetRotation()const { return rotation; }
	const Vector2& GetSize()const { return size; }
	const Vector4& GetColor()const { return materialData->color; }
	const Vector2& GetAnchorPoint()const { return anchorPoint; }
	bool GetFlipX()const { return isFlipX_; }
	bool GetFlipY()const { return isFlipY_; }
	const Vector2& GetTextureLeftTop()const { return textureLeftTop; }
	const Vector2& GetTextureSize()const { return textureSize; }
	

public://セッター
	void SetPosition(const Vector2& position) { this->position = position; }
	void SetRotation(float rotation) { this->rotation = rotation; }
	void SetSize(const Vector2& size) { this->size = size; }
	void SetColor(const Vector4& color) { materialData->color = color; }
	void SetAnchorPoint(const Vector2& anchorPoint) { this->anchorPoint = anchorPoint; }
	void SetFlipX(bool isFlipX) { isFlipX_ = isFlipX; }
	void SetFlipY(bool isFlipY) { isFlipY_ = isFlipY; }
	void SetTextureLeftTop(const Vector2& leftTop) { textureLeftTop = leftTop; }
	void SetTextureSize(const Vector2& size) { textureSize = size; }

private://メンバ変数
	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource = nullptr;
	//バッファリソース内のデータを指すポインタ
	Struct::VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	Struct::DirectionalLight* directionalLightData = nullptr;
	Struct::Material* materialData = nullptr;
	Struct::TransformationMatrix* transformationMatrixData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	//UVトランスフォーム
	Transform uvTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};
	//テクスチャ設定
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;

private://メンバ変数値書き換え用
	Vector2 position = { 0.0f,0.0f };
	float rotation = 0.0f;
	Vector2 size = { 640.0f,360.0f };
	Vector2 anchorPoint = { 0.0f,0.0f };
	//左右フリップ
	bool isFlipX_ = false;
	//上下フリップ
	bool isFlipY_ = false;
	//テクスチャ左上座標
	Vector2 textureLeftTop = { 0.0f,0.0f };
	//テクスチャ切り出しサイズ
	Vector2 textureSize = { 100.0f,100.0f };
	//テクスチャファイルパス
	std::string textureFilePath_;

};

