#pragma once
#include <wrl.h>
#include <d3d12.h>

class ShapeCommon
{
private://コンストラクタ等の隠蔽
	static ShapeCommon* instance;

	ShapeCommon() = default;//コンストラクタ隠蔽
	~ShapeCommon() = default;//デストラクタ隠蔽
	ShapeCommon(ShapeCommon&) = delete;//コピーコンストラクタ封印
	ShapeCommon& operator=(ShapeCommon&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static ShapeCommon* GetInstance();
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

public://セッター

private://インスタンス

private://メンバ変数
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

};

