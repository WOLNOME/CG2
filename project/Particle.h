#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <list>
#include <memory>
#include <numbers>
#include "Model.h"
#include "Function.h"
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class Particle
{
public://インナークラス
	class Struct {
	public:
		//平行光源データ
		struct DirectionalLight
		{
			Vector4 color;
			Vector3 direction;
			float intensity;
		};
		//座標変換行列データ
		struct ParticleForGPU {
			Matrix4x4 WVP;
			Matrix4x4 World;
			Vector4 color;
		};
		//モデルリソース作成用データ型
		struct ParticleResource {
			Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
			ParticleForGPU* instancingData;
			Transform transform;
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
		//フィールド
		struct AccelerationField
		{
			Vector3 acceleration;
			AABB area;
		};

	};
public://メンバ関数
	//初期化
	void Initialize(const std::string& filePath);
	void Update();
	void Draw();
private://メンバ関数(非公開)
	//パーティクルリソース作成関数
	Struct::ParticleResource MakeParticleResource();
	//SRVの設定
	void SettingSRV();
	//パーティクルの生成
	Struct::Particle MakeNewParticle(const Vector3& translate);
	//エミット
	std::list<Struct::Particle> Emit(const Struct::Emitter& emitter);

private://インスタンス
private://メンバ変数
	//モデル(見た目)
	Model* model_;
	//パーティクル用リソース
	Struct::ParticleResource particleResource_;

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
	//フィールド
	Struct::AccelerationField accelerationField;
	//フィールド値のオンオフ
	bool isField = false;
};