#pragma once
#include <wrl.h>
#include <d3d12.h>

class SkyboxCommon {
private: // シングルトン
	static SkyboxCommon* instance;
	SkyboxCommon() = default; // コンストラクタ隠蔽
	~SkyboxCommon() = default; // デストラクタ隠蔽
	SkyboxCommon(SkyboxCommon&) = delete; // コピーコンストラクタ封印
	SkyboxCommon& operator=(SkyboxCommon&) = delete; // コピー代入演算子封印
public: // シングルトン
	static SkyboxCommon* GetInstance();
public: // メンバ関数
	// 初期化
	void Initialize();
	// 終了
	void Finalize();
	// 共通描画設定
	void SettingCommonDrawing();
private: // 非公開メンバ関数
	// グラフィックスパイプライン
	void GenerateGraphicsPipeline();
public: // ゲッター
public: // セッター
private: // メンバ変数
	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState;

};

