#pragma once
#include <wrl.h>
#include <d3d12.h>

class SpriteCommon
{
private://シングルトン
	static SpriteCommon* instance;

	SpriteCommon() = default;//コンストラクタ隠蔽
	~SpriteCommon() = default;//デストラクタ隠蔽
	SpriteCommon(SpriteCommon&) = delete;//コピーコンストラクタ封印
	SpriteCommon& operator=(SpriteCommon&) = delete;//コピー代入演算子封印
public://シングルトン
	static SpriteCommon* GetInstance();
public://メンバ関数
	//初期化
	void Initialize();
	//終了
	void Finalize();
	//共通描画設定
	void SettingCommonDrawing();

private://非公開メンバ関数(内部処理)
	//グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();
public://ゲッター
	
private://インスタンス
	
private://メンバ関数
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	
};

