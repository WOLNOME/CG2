#pragma once
#include "wrl.h"
#include "d3d12.h"
#include"Function.h"

class SpriteCommon;

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
	void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);
	void Update();
	void Draw();
public://ゲッター
	const Vector2& GetPosition()const { return position; }
	float GetRotation()const { return rotation; }
	const Vector2& GetSize()const { return size; }
	const Vector4& GetColor()const { return materialData->color; }


public://セッター
	void SetPosition(const Vector2& position) { this->position = position; }
	void SetRotation(float rotation) { this->rotation = rotation; }
	void SetSize(const Vector2& size) { this->size = size; }
	void SetColor(const Vector4& color) { materialData->color = color; }


private://インスタンス
	SpriteCommon* spriteCommon_ = nullptr;
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
	//テクスチャ番号
	uint32_t textureIndex = 0;
};

