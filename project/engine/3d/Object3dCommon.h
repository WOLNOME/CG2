#pragma once
#include <wrl.h>
#include <d3d12.h>

class Camera;

//モデル共通部
class Object3dCommon
{
private://シングルトン
	static Object3dCommon* instance;

	Object3dCommon() = default;//コンストラクタ隠蔽
	~Object3dCommon() = default;//デストラクタ隠蔽
	Object3dCommon(Object3dCommon&) = delete;//コピーコンストラクタ封印
	Object3dCommon& operator=(Object3dCommon&) = delete;//コピー代入演算子封印
public://シングルトン
	static Object3dCommon* GetInstance();
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

