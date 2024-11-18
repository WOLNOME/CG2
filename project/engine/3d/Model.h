#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include "MyMath.h"

class Model
{
private://インナークラス
	class Struct {
	public:
		//頂点データ
		struct VertexData {
			Vector4 position;
			Vector2 texcoord;
			Vector3 normal;
		};
		//マテリアル
		struct Material {
			Vector4 color;
			int32_t lightingKind;
			float padding[3];//(PSに送るときは16バイトに揃える)
			Matrix4x4 uvTransform;
			int32_t isTexture;
		};
		//マテリアルデータ
		struct MaterialData {
			std::string textureFilePath;
			Vector4 colorData;
			uint32_t textureIndex = 0;
		};
		//モデルデータ
		struct ModelData {
			std::vector<VertexData> vertices;
			MaterialData material;
			std::string materialName;
		};
		//モデルリソース作成用データ型
		struct ModelResource {
			std::vector<ModelData> modelData;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResource;
			std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView;
			std::vector<VertexData*> vertexData;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResource;
			std::vector<Material*> materialData;
			std::vector<Transform> uvTransform;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResorce;
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU;
			std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU;
		};
	};
public:
	void Initialize(const std::string& directorypath, const std::string& filename);
	void Update();
	void Draw(uint32_t materialRootParameterIndex, uint32_t textureRootParameterIndex, uint32_t instancingNum = 1);

public://ゲッター
public://セッター

private:
	//.mtlファイルの読み取り
	Struct::MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName, const std::string& materialName);
	//.objファイルの読み取り
	std::vector<Struct::ModelData> LoadObjFile(const std::string& directoryPath, const std::string& fileName);
	//モデルリソース作成関数
	Struct::ModelResource MakeModelResource(const std::string& resourceFileName, const std::string& objFileName);
	//テクスチャ読み込み
	void SettingTexture();
private:
	//モデル用リソース
	Struct::ModelResource modelResource_;
	//モデル数
	size_t modelNum_;

};

