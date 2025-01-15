#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <list>
#include <memory>
#include <numbers>
#include "Model.h"
#include "MyMath.h"

class BaseCamera;
//パーティクル
class Particle
{
public:
	//座標変換行列データ
	struct ParticleForGPU {
		Matrix4x4 World;
		Vector4 color;
	};
	//モデルリソース作成用データ型
	struct ParticleResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
		ParticleForGPU* instancingData;
		TransformEuler transform;
		D3D12_CPU_DESCRIPTOR_HANDLE SrvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE SrvHandleGPU;
		uint32_t srvIndex;
	};
	//パーティクル構造体
	struct ParticleData {
		TransformEuler transform;
		Vector3 velocity;
		Vector4 color;
		float lifeTime;
		float currentTime;
	};
	//エミッター構造体
	struct Emitter {
		TransformEuler transform;//エミッターのトランスフォーム
		uint32_t count;//発生させるパーティクルの数
		float frequency;//発生頻度
		float frequencyTime;//頻度用時刻
	};
	//フィールド
	struct AccelerationField
	{
		Vector3 acceleration;
		AABB area;
		bool isActive;
	};
public://メンバ関数
	~Particle();
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="filePath">オブジェクトファイルパス(.objはいらない)</param>
	void Initialize(const std::string& filePath);
	void Draw(const BaseCamera& camera, Emitter& emitter, AccelerationField* field = nullptr);
private://メンバ関数(非公開)
	//パーティクルリソース作成関数
	ParticleResource MakeParticleResource();
	//SRVの設定
	void SettingSRV();
	//パーティクルの生成
	ParticleData MakeNewParticle(const Vector3& translate);
	//エミット
	std::list<ParticleData> Emit(const Emitter& emitter);

private://インスタンス
private://メンバ変数
	//モデル(見た目)
	Model* model_;
	//パーティクル用リソース
	ParticleResource particleResource_;

	//各インスタンシング用書き換え情報
	std::list<ParticleData> particles;
	//表示するパーティクルの最大数
	const uint32_t kNumMaxInstance_ = 64;
	//δtの定義
	const float kDeltaTime = 1.0f / 60.0f;
	//ビルボードのオンオフ
	bool isBillboard = true;

};