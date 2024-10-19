#pragma once
#include <wrl.h>
#include <d3d12.h>

class DirectXCommon;

//モデル共通部
class ModelCommon
{
public://メンバ関数
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//共通描画設定
	void SettingCommonDrawing();
private://非公開メンバ関数
	//グラフィックスパイプライン
	void GenerateGraphicsPipeline();
public://ゲッター
	DirectXCommon* GetDxCommon()const { return dxCommon_; }


private://インスタンス
	DirectXCommon* dxCommon_ = nullptr;

private://メンバ変数
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;


};

