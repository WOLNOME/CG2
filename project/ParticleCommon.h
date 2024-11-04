#pragma once
#include <wrl.h>
#include <d3d12.h>

class Camera;
class ParticleCommon
{
private://コンストラクタ等の隠蔽
	static ParticleCommon* instance;

	ParticleCommon() = default;//コンストラクタ隠蔽
	~ParticleCommon() = default;//デストラクタ隠蔽
	ParticleCommon(ParticleCommon&) = delete;//コピーコンストラクタ封印
	ParticleCommon& operator=(ParticleCommon&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static ParticleCommon* GetInstance();
public://メンバ関数
	//初期化
	void Initialize();
	//終了
	void Finalize();
	//共通描画設定
	void SettingCommonDrawing();
private://非公開メンバ関数
	//グラフィックスパイプライン
	void GenerateGraphicsPipeline();

public://ゲッター
	Camera* GetDefaultCamera()const { return defaultCamera; }
public://セッター
	void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }
private://インスタンス
	Camera* defaultCamera = nullptr;
private://メンバ変数
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

};