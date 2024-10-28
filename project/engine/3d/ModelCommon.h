#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "Vector4.h"
#include "Vector3.h"

class DirectXCommon;
class Camera;

//モデル共通部
class ModelCommon
{
private://シングルトン設定
	static ModelCommon* instance;

	ModelCommon() = default;//コンストラクタ隠蔽
	~ModelCommon() = default;//デストラクタ隠蔽
	ModelCommon(ModelCommon&) = delete;//コピーコンストラクタ封印
	ModelCommon& operator=(ModelCommon&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static ModelCommon* GetInstance();
private://インナークラス
	//平行光源データ
	struct DirectionalLight
	{
		Vector4 color;
		Vector3 direction;
		float intensity;
	};
public://メンバ関数
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//終了
	void Finalize();
	//描画
	void Draw();

	//共通描画設定
	void SettingCommonDrawing();
private://非公開メンバ関数
	//グラフィックスパイプライン
	void GenerateGraphicsPipeline();
	//平行光源の生成
	void GenerateDirectionalLight();

public://ゲッター
	DirectXCommon* GetDirectXCommon()const { return dxCommon_; }
	Camera* GetDefaultCamera()const { return defaultCamera; }
public://セッター
	void SetDefaultCamera(Camera* camera) { defaultCamera = camera; }

private://インスタンス
	DirectXCommon* dxCommon_ = nullptr;
	Camera* defaultCamera = nullptr;
private://メンバ変数
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
private://オブジェクト共通の設定関連
	//平行光源用バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	//平行光源用バッファリソース内のデータをさすポインタ
	DirectionalLight* directionalLightData = nullptr;


};

