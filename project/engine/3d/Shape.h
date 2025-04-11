#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "MyMath.h"

class Shape
{
public://列挙型
	enum ShapeKind
	{
		kSphere,
		kCube,
	};
private:
	//頂点データ
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	//マテリアル
	struct Material {
		Vector4 color;
		Matrix4x4 uvTransform;
		float isTexture;
		float shininess;
	};
	//形状リソース作成用データ型
	struct ShapeResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		VertexData* vertexData;
		uint32_t vertexNum;
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
		Material* materialData;
	};

public:
	void Initialize(ShapeKind shapeKind);
	void Update();
	void Draw(uint32_t materialRootParameterIndex, uint32_t textureRootParameterIndex, int32_t textureHandle = EOF);

private://非公開メンバ関数
	//形状リソース作成関数
	ShapeResource MakeShapeResource();
private:
	//形状の種類
	ShapeKind shapeKind_;
	//形状用リソース
	ShapeResource shapeResource_;
	//球体の分割数
	const uint32_t kSubdivision = 15;

};

