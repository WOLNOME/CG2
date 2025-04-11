#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <list>
#include <memory>
#include "json.hpp"
#include "Model.h"
#include "MyMath.h"

using json = nlohmann::json;

//パーティクル
class Particle {
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
		uint32_t srvIndex;
	};
	//エフェクト構造体
	struct EffectData {
		TransformEuler transform;	//エフェクトのトランスフォーム
		Vector4 startColor;			//最初の色
		Vector4 endColor;			//最後の色
		Vector3 velocity;			//速度
		float startSize;			//最初のサイズ
		float endSize;				//最後のサイズ
		float lifeTime;				//寿命
		float currentTime;			//現在の時間
	};
	//エミッター
	struct Emitter {
		TransformEuler transform;	//エミッターのトランスフォーム
		float gravity;				//重力値
		float repulsion;			//床の反発値
		float floorHeight;			//床の高さ
		bool isAffectedField;		//フィールドに影響を受けるか
		bool isBillboard;			//ビルボードを適用するか
		bool isGravity;				//重力を適用するか
		bool isBound;				//バウンドを適用するか
		bool isPlay;				//パーティクルを生成するか
	};
public://メンバ関数
	~Particle();
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="name">インスタンスの名前</param>
	/// <param name="fileName">使用するパーティクルの名前(.jsonは省略)</param>
	void Initialize(const std::string& name,const std::string& fileName);
private://メンバ関数(非公開)
	//パーティクルリソース作成関数
	ParticleResource MakeParticleResource();
	//SRVの設定
	void SettingSRV();

public: //getter
	//パラメーター
	const json& GetParam() { return param_; }
public: //setter
	//パラメーター
	void SetParam(const json& param) { param_ = param; }
public: //マネージャー共有用変数
	//モデル(見た目)
	Model* model_;
	//パーティクル用リソース
	ParticleResource particleResource_;
	//各インスタンシング（エフェクト）用書き換え情報
	std::list<EffectData> effects_;
public://エミッター
	Emitter emitter_;
private: //メンバ変数
	//インスタンスの名前
	std::string name_;
	//各エフェクトのパラメーター
	json param_;

};