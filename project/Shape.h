#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <vector>
#include "Vector3.h"
#include "Vector4.h"
#include "WorldTransform.h"
#include "SceneLight.h"

class BaseCamera;
class Shape
{
public://列挙型
	enum ShapeKind {
		kSphere,
		kCube,
		kSkyBox,
	};

public://構造体
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
	//リソース作成用データ型
	struct ShapeResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		VertexData* vertexData;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		std::vector<uint32_t> indexData;
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
		Material* materialData;
	};
public://メンバ関数
	Shape();
	~Shape();

	//初期化
	void Initialize(ShapeKind kind);
	void Draw(const WorldTransform& worldTransform, const BaseCamera& camera, const SceneLight* sceneLight = nullptr);

private://非公開メンバ関数
	//パーティクルリソース作成関数
	ShapeResource MakeShapeResource();
private://インスタンス
private://メンバ変数
	//パーティクル用リソース
	ShapeResource shapeResource_;
	//形状の種類
	ShapeKind kind_;
};

