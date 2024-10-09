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
	//PSOの設定
	void SettingPSO();
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
	//インプットレイアウト
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	//頂点シェーダー
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
	//ピクセルシェーダー
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;
	//blendState
	D3D12_BLEND_DESC blendDesc{};
	//RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//DepthStencilState
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

};

