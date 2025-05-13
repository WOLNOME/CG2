#pragma once
#include "BaseCamera.h"
#include "Particle.h"
#include "MyMath.h"
#include "BlendMode.h"
#include <d3d12.h>
#include <string>
#include <array>
#include <list>
#include <unordered_map>
#include <wrl.h>

class ParticleManager {
public:
	//フィールド
	struct AccelerationField {
		Vector3 acceleration;
		AABB area;
		bool isActive;
	};
private://コンストラクタ等の隠蔽
	static ParticleManager* instance;

	ParticleManager() = default;//コンストラクタ隠蔽
	~ParticleManager() = default;//デストラクタ隠蔽
	ParticleManager(ParticleManager&) = delete;//コピーコンストラクタ封印
	ParticleManager& operator=(ParticleManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static ParticleManager* GetInstance();
public://メンバ関数
	//初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	//終了
	void Finalize();

	//パーティクルをコンテナに登録
	void RegisterParticle(const std::string& name, Particle* particle);
	//登録されたパーティクルを削除
	void DeleteParticle(const std::string& name);

public://パーティクルコンテナの操作


private://パーティクル全体の操作
	//グラフィックスパイプライン
	void GenerateGraphicsPipeline();
private://粒の操作
	//粒の生成
	std::list<Particle::GrainData> GenerateGrain(Particle* particle, int genNum);

public://ゲッター
public://セッター
	void SetCamera(BaseCamera* camera) { camera_ = camera; }
private://インスタンス
	BaseCamera* camera_ = nullptr;
private://メンバ変数
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, (int)BlendMode::kMaxBlendModeNum> graphicsPipelineState;

	//パーティクルのコンテナ
	std::unordered_map<std::string, Particle*> particles;

	//共通フィールド
	AccelerationField* field_ = nullptr;

};

