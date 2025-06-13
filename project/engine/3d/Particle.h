#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <list>
#include <memory>
#include "json.hpp"
#include "Shape.h"
#include "MyMath.h"

using json = nlohmann::json;

//パーティクル
class Particle {
	//パーティクルマネージャーに公開
	friend class ParticleManager;
	//パーティクルクリエイターシーンに公開
	friend class ParticleCreatorScene;
public:
	//生成方法
	enum class GenerateMethod {
		Random,		//ランダム
		Clump,		//塊	
	};
	//エフェクトスタイル
	enum class EffectStyle {
		Loop,		//ループ(永続)
		OneShot,	//一度きり
	};

private://非公開構造体
	///========================///
	///		CSで使用する構造体
	///========================///
	//CS用トランスフォーム(paddingの手間を防ぐため)
	struct TransformForCS {
		Vector4 scale;
		Vector4 rotate;
		Vector4 translate;
	};
	//CS用粒の情報
	struct GrainForCS {
		TransformForCS transform;
		TransformForCS basicTransform;
		Vector4 velocity;
		Vector4 startColor;
		Vector4 endColor;
		Vector4 startRotate;
		Vector4 endRotate;
		float startSize;
		float endSize;
		float lifeTime;
		float currentTime;
	};
	//CS用エミッター情報
	struct EmitterForCS {
		TransformForCS transform;
		int generateMethod;
		int effectStyle;
		float gravity;
		float repulsion;
		float floorHeight;
		int clumpNum;
		uint32_t isAffectedField;
		uint32_t isGravity;
		uint32_t isBound;
		uint32_t isBillboard;
		uint32_t isPlay;
	};
	//CS用Json情報
	struct JsonInfoForCS {
		Vector4 velocityMax;
		Vector4 velocityMin;
		Vector4 initRotateMax;
		Vector4 initRotateMin;
		Vector4 initScaleMax;
		Vector4 initScaleMin;
		Vector4 startColorMax;
		Vector4 startColorMin;
		Vector4 endColorMax;
		Vector4 endColorMin;
		Vector4 startRotateMax;
		Vector4 startRotateMin;
		Vector4 endRotateMax;
		Vector4 endRotateMin;
		float startSizeMax;
		float startSizeMin;
		float endSizeMax;
		float endSizeMin;
		float lifeTimeMax;
		float lifeTimeMin;
		int emitRate;
		int maxGrains;
	};
	//CS用時間情報
	struct PerFrameForCS {
		float time;
		float deltaTime;
	};
	//CS用リソースのまとめ
	struct AllResourceForCS {
		//粒の情報
		Microsoft::WRL::ComPtr<ID3D12Resource> grainsResource;
		uint32_t grainsSrvIndex;	//VS用
		uint32_t grainsUavIndex;	//CS用
		//フリーリストのインデックス情報
		Microsoft::WRL::ComPtr<ID3D12Resource> freeListIndexResource;
		uint32_t freeListIndexUavIndex;		//CS用
		//フリーリストの情報
		Microsoft::WRL::ComPtr<ID3D12Resource> freeListResource;
		uint32_t freeListUavIndex;		//CS用
		//エミッター情報
		Microsoft::WRL::ComPtr<ID3D12Resource> emitterResource;
		std::span<EmitterForCS> mappedEmitter;
		//JSON情報
		Microsoft::WRL::ComPtr<ID3D12Resource> jsonInfoResource;
		std::span<JsonInfoForCS> mappedJsonInfo;
		//時間情報
		Microsoft::WRL::ComPtr<ID3D12Resource> perFrameResource;
		std::span<PerFrameForCS> mappedPerFrame;
	};

public://公開構造体
	//エミッター
	struct Emitter {
		TransformEuler transform;			//エミッターのトランスフォーム
		GenerateMethod generateMethod;		//生成方法
		EffectStyle effectStyle;			//エフェクトスタイル
		float gravity;						//重力値
		float repulsion;					//床の反発値
		float floorHeight;					//床の高さ
		int clumpNum;						//塊の数(clumpMethodの時のみ使用)
		bool isAffectedField;				//フィールドに影響を受けるか
		bool isBillboard;					//ビルボードを適用するか
		bool isGravity;						//重力を適用するか
		bool isBound;						//バウンドを適用するか
		bool isPlay;						//パーティクルを生成するか
	};
public://メンバ関数
	~Particle();
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="name">インスタンスの名前</param>
	/// <param name="fileName">使用するパーティクルの名前(.jsonは省略)</param>
	void Initialize(const std::string& name, const std::string& fileName);
private://メンバ関数(非公開)
	//CS専用リソースの作成
	AllResourceForCS CreateAllResourceForCS();

	//形状の変更
	void ShapeChange();
	//テクスチャの変更
	void TextureChange();

	//エミッター反映(CSに反映)
	void TraceEmitterForCS();
	//JSONデータ反映(CSに反映)
	void TraceJsonDataForCS();

public: //getter
	//パラメーター
	const json& GetParam() { return param_; }
public: //setter
	//パラメーター
	void SetParam(const json& param) { param_ = param; }

public://公開パラメーター
	Emitter emitter_;

private: //メンバ変数(非公開)
	//形状(見た目)
	std::unique_ptr<Shape> shape_;
	//CS専用のリソース
	AllResourceForCS allResourceForCS_;

	//インスタンスの名前
	std::string name_;
	//各粒のパラメーター
	json param_;
	//テクスチャハンドル
	int32_t textureHandle_;

};