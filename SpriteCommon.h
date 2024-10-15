#pragma once
#include "DirectXCommon.h"

class SpriteCommon
{
public://メンバ関数
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//共通描画設定
	void SettingCommonDrawing();

private://非公開メンバ関数(内部処理)
	//グラフィックスパイプラインの生成
	void GenerateGraphicsPipeline();
public://ゲッター
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }
private://インスタンス
	DirectXCommon* dxCommon_ = nullptr;
private://メンバ関数
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	
};

