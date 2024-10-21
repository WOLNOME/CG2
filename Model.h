#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include "Function.h"

class ModelCommon;
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
			float padding[3];
			Matrix4x4 uvTransform;
			int32_t isTexture;
		};
		//座標変換行列データ
		struct TransformationMatrix {
			Matrix4x4 WVP;
			Matrix4x4 World;
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
			Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
			TransformationMatrix* wvpData;
			Transform transform;
			std::vector<Transform> uvTransform;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResorce;
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU;
			std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU;
		};
	};
public:
	void Initialize(ModelCommon* modelCommon,const std::string& directorypath,const std::string& filename);
	void Update();
	void Draw();
public://ゲッター
	const Vector3& GetScale() { return modelResource_.transform.scale; }
	const Vector3& GetRotate() { return modelResource_.transform.rotate; }
	const Vector3& GetTranslate() { return modelResource_.transform.translate; }
public://セッター
	void SetScale(const Vector3& scale) { modelResource_.transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { modelResource_.transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { modelResource_.transform.translate = translate; }

private:
	//.mtlファイルの読み取り
	static Struct::MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName, const std::string& materialName);
	//.objファイルの読み取り
	static std::vector<Struct::ModelData> LoadObjFile(const std::string& directoryPath, const std::string& fileName);
	//モデルリソース作成関数
	Struct::ModelResource MakeModelResource(const std::string& resourceFileName, const std::string& objFileName);
	//テクスチャ読み込み
	void SettingTexture();
private:
	ModelCommon* modelCommon_;
	//モデル用リソース
	Struct::ModelResource modelResource_;

	//カメラトランスフォーム
	Transform cameraTransform = {
		{1.0f,1.0f,1.0f},
		{0.3f,0.0f,0.0f},
		{0.0f,4.0f,-10.0f}
	};
};

