#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <list>
#include <memory>
#include <numbers>
#include "Function.h"
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class ParticleCommon;
class Model;
class Particle
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
		//マテリアル
		struct Material {
			Vector4 color;
			int32_t lightingKind;
			float padding[3];
			Matrix4x4 uvTransform;
			int32_t isTexture;
		};
		//座標変換行列データ
		struct ParticleForGPU {
			Matrix4x4 WVP;
			Matrix4x4 World;
			Vector4 color;
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
			Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
			ParticleForGPU* instancingData;
			Transform transform;
			std::vector<Transform> uvTransform;
			std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResorce;
			std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU;
			std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU;
		};
		//パーティクル構造体
		struct Particle {
			Transform transform;
			Vector3 velocity;
			Vector4 color;
			float lifeTime;
			float currentTime;
		};
		//エミッター構造体
		struct Emitter {
			Transform transform;//エミッターのトランスフォーム
			uint32_t count;//発生させるパーティクルの数
			float frequency;//発生頻度
			float frequencyTime;//頻度用時刻
		};

	};
public://メンバ関数
	//初期化
	void Initialize(ParticleCommon* particleCommon);
	void Update();
	void Draw();
	//.mtlファイルの読み取り
	static Struct::MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName, const std::string& materialName);
	//.objファイルの読み取り
	static std::vector<Struct::ModelData> LoadObjFile(const std::string& directoryPath, const std::string& fileName);
private://メンバ関数(非公開)
	//モデルリソース作成関数
	void MakeModelResource(const std::string& resourceFileName, const std::string& objFileName);
	//テクスチャ読み込み
	void SettingTexture();
	//SRVの設定
	void SettingSRV();
	//パーティクルの生成
	Struct::Particle MakeNewParticle(const Vector3& translate);
	//エミット
	std::list<Struct::Particle> Emit(const Struct::Emitter& emitter);

private://インスタンス
	ParticleCommon* particleCommon_ = nullptr;
private://メンバ変数
	//モデル用リソース
	Struct::ModelResource particleResource_;
	//平行光源用バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	//平行光源用バッファリソース内のデータをさすポインタ
	Struct::DirectionalLight* directionalLightData = nullptr;
	//カメラトランスフォーム
	Transform cameraTransform = {
		{1.0f,1.0f,1.0f},
		{std::numbers::pi_v<float> / 3.0f,std::numbers::pi_v<float>,0.0f},
		{0.0f,23.0f,10.0f}
	};
	//各インスタンシング用書き換え情報
	std::list<Struct::Particle> particles;
	//srvハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE SrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE SrvHandleGPU;
	//表示するパーティクルの最大数
	const uint32_t kNumMaxInstance_ = 64;
	//δtの定義
	const float kDeltaTime = 1.0f / 60.0f;
	//ビルボードのオンオフ
	bool isBillboard = false;
	//エミッター
	Struct::Emitter emitter{};
};

