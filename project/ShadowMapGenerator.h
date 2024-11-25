#pragma once
#include <d3d12.h>
#include <wrl.h>


class ShadowMapGenerator
{
private://コンストラクタ等の隠蔽
	static ShadowMapGenerator* instance;

	ShadowMapGenerator() = default;//コンストラクタ隠蔽
	~ShadowMapGenerator() = default;//デストラクタ隠蔽
	ShadowMapGenerator(ShadowMapGenerator&) = delete;//コピーコンストラクタ封印
	ShadowMapGenerator& operator=(ShadowMapGenerator&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static ShadowMapGenerator* GetInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();
	//シャドウマップ共通描画設定
	void SettingCommonDrawing();
private://非公開メンバ関数
	//グラフィックスパイプライン
	void GenerateGraphicsPipeline();

public://ゲッター
public://セッター

private://メンバ変数
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

};

