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

	//名前を決める関数
	std::string GenerateName(const std::string& name);

private://パーティクル全体の操作
	//グラフィックスパイプライン
	void GenerateGraphicsPipeline();
	//コンピュートパイプライン
	void GenerateComputePipeline();

	//初期化用CPSOの設定
	void InitCPSOOption();
	//エミット用CPSOの設定
	void EmitCPSOOption();
	//更新用CPSOの設定
	void UpdateCPSOOption();

public://セッター
	void SetCamera(BaseCamera* camera) { camera_ = camera; }
private://インスタンス
	BaseCamera* camera_ = nullptr;
private://メンバ変数
	//Gルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> gRootSignature = nullptr;
	//グラフィックスパイプライン
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, (int)BlendMode::kMaxBlendModeNum> graphicsPipelineState;
	//Cルートシグネチャ(init,emit,update分あるので3つ)
	std::array<Microsoft::WRL::ComPtr<ID3D12RootSignature>, 3> cRootSignature;
	//コンピュートパイプライン
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, 3> computePipelineState;

	//パーティクルのコンテナ
	std::unordered_map<std::string, Particle*> particles;

};

