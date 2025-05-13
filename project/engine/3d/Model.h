#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include "MyMath.h"
#include "ModelFormat.h"
//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
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
	//ノード
	struct Node {
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};
	//マテリアルデータ
	struct MaterialData {
		std::string materialName;
		std::string textureFilePath;
		Vector4 colorData;
		uint32_t textureHandle;
	};
	//モデルデータ
	struct ModelData {
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		MaterialData material;
		Node rootNode;
	};
	//モデルリソース作成用データ型
	struct ModelResource {
		std::vector<ModelData> modelData;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResource;
		std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView;
		std::vector<VertexData*> vertexData;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResource;
		std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferView;
		std::vector<uint32_t*> indexData;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResource;
		std::vector<Material*> materialData;
		std::vector<TransformEuler> uvTransform;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResorce;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU;
	};

public:
	void Initialize(const std::string& filename, ModelFormat format = OBJ, std::string directorypath = "Resources/models/");
	void Draw(uint32_t materialRootParameterIndex, uint32_t textureRootParameterIndex, uint32_t instancingNum = 1, int32_t textureHandle = -1);

public://ゲッター
	const ModelResource& GetModelResource() { return modelResource_; }
public://セッター
	void SetColor(Vector4& color) { color_ = &color; }

private:
	//モデルファイルの読み取り
	std::vector<ModelData> LoadModelFile();
	//assimpのノード→構造体ノード変換関数
	Node ReadNode(aiNode* node);
	//モデルリソース作成関数
	ModelResource MakeModelResource();
	//テクスチャ読み込み
	void SettingTexture();
private:
	//モデル用リソース
	ModelResource modelResource_;
	//モデル数
	size_t modelNum_;
	//色
	Vector4* color_ = nullptr;

	//ディレクトリパス
	std::string directoryPath_;
	//モデルデータファイル名
	std::string fileName_;
	//形式名
	ModelFormat mf_;
	std::string format_;
};