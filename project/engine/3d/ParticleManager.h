#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <wrl.h>
#include <d3d12.h>
#include <random>
#include <cstdint>
#include <string>
#include <list>
#include <numbers>
#include <unordered_map>
#include "Model.h"
#include "BaseCamera.h"


class DirectXCommon;
class SrvManager;
class ParticleManager
{
private://シングルトン
	static ParticleManager* instance;

	ParticleManager() = default;//コンストラクタ隠蔽
	~ParticleManager() = default;//デストラクタ隠蔽
	ParticleManager(ParticleManager&) = delete;//コピーコンストラクタ封印
	ParticleManager& operator=(ParticleManager&) = delete;//コピー代入演算子封印
public://シングルトン
	static ParticleManager* GetInstance();
public://構造体
	class Struct
	{
	public:
		//パーティクル
		struct Particle {
			TransformEuler transform;
			Vector3 velocity;
			Vector4 color;
			float lifeTime;
			float currentTime;
		};
		//座標変換行列データ
		struct ParticleForGPU {
			Matrix4x4 WVP;
			Matrix4x4 World;
			Vector4 color;
		};
		//パーティクルグループ
		struct ParticleGroup {
			Model::MaterialData materialData;
			std::list<Particle> particles;
			uint32_t instancingSrvIndex;
			Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
			uint32_t instanceNum;
			ParticleForGPU* instancingData;
			Model* model;
		};
		//フィールド
		struct AccelerationField
		{
			Vector3 acceleration;
			AABB area;
		};
	};
public://メンバ関数
	//初期化
	void Initialize();
	//更新
	void Update(const BaseCamera* camera);
	//描画
	void Draw(const BaseCamera* camera);
	//終了
	void Finalize();
	//パーティクルの発生
	void Emit(const std::string name, const Vector3& position, uint32_t count);
	//パーティクルグループの生成
	void CreateParticleGroup(const std::string& name, const std::string& modelFilePath);
	//パーティクルのテクスチャ変更
	void SetTexture(const std::string& name, const std::string& textureFilePath);
private://非公開メンバ関数
	//グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();
	//パーティクルの生成
	Struct::Particle MakeNewParticle(const Vector3& translate);
public://ゲッター
	std::unordered_map<std::string, Struct::ParticleGroup> GetParticleGroups() { return particleGroups_; }

private://インスタンス
	SrvManager* srvManager_ = nullptr;
private://メンバ変数
	//ランダムエンジン
	std::mt19937 randomEngine_;
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	//パーティクルグループコンテナ
	std::unordered_map < std::string, Struct::ParticleGroup> particleGroups_;
	//インスタンスの最大値
	const uint32_t kNumMaxInstance_ = 100;
	//フィールド
	Struct::AccelerationField accelerationField;
	//δtの定義
	const float kDeltaTime = 1.0f / 60.0f;
	//テクスチャハンドル
	int32_t textureHandle_=EOF;
};

