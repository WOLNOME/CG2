#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <list>
#include <vector>
#include "Matrix4x4.h"

//シャドウマップレンダリング時に使う定数バッファ
struct LightViewProjectionForVS {
	Matrix4x4 viewProjectionMatrix;
};

class SceneLight;
class ShadowMapManager
{
private://コンストラクタ等の隠蔽
	static ShadowMapManager* instance;

	ShadowMapManager() = default;//コンストラクタ隠蔽
	~ShadowMapManager() = default;//デストラクタ隠蔽
	ShadowMapManager(ShadowMapManager&) = delete;//コピーコンストラクタ封印
	ShadowMapManager& operator=(ShadowMapManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static ShadowMapManager* GetInstance();
public:
	//スポットライト全てのSM関連情報を格納する構造体
	struct SLShadowMapInfo {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;		//リソース(スポットライトの数はTexture2DArrayのサイズ)
		std::list<uint32_t> dsvIndex;							//DSVハンドル
		uint32_t srvIndex;										//SRVインデックス
	};
public:
	//レンダーループからの脱出フラグ
	bool isEscapeLoop = false;
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();
	//シャドウマップ共通描画設定
	void SettingCommonDrawing();
	//どのリソースをバリアに貼るのかを選択
	bool SelectResource(const SceneLight* sceneLight);
	//描画前設定
	uint32_t PreDraw();
	//描画後設定
	void PostDraw();
	//SL用SM情報の取得
	const SLShadowMapInfo& GetSLSMInfo() { return slsmInfo; }
	/// <summary>
	/// 選択済みのライトビュープロジェクション定数バッファ
	/// </summary>
	/// <returns></returns>
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetLightViewProjectionConstBuffer(int index) const { return lightViewProjectionResources_[index]; }

private://非公開メンバ関数
	//ライト視点のVPMの初期化
	void InitLVPM();
	//グラフィックスパイプライン
	void GenerateGraphicsPipeline();
	//SM関連情報を登録する関数
	void InitShadowMapInfo();
	//Texture2D用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> MakeTexture2DResource(int width, int height);
	//Texture2DArray用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> MakeTexture2DArrayResource(int width, int height, int numElements);
	//DSV関連の設定
	void SettingDSV();
	//SRV関連の設定
	void SettingSRV();

public://ゲッター
public://セッター

private://メンバ変数
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	//DSVインデックスの指標(DSVマネージャーを作ったらこれも管理できる)
	int dsvIndexCount = 0;

	//SL用SM情報
	SLShadowMapInfo slsmInfo;

	//TransitionBarrier
	D3D12_RESOURCE_BARRIER barrier{};

	//シャドウマップに送る用の各ライトの視点用定数バッファ
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> lightViewProjectionResources_;
	//シャドウマップに送る用の各ライトの視点用マッピング済みアドレス
	std::vector<LightViewProjectionForVS*> lightViewProjectionDatas_;
	//ライトVPMの場所を割り当てる変数
	uint32_t lVPMIndex = -1;

	//バリアを貼るためのリソースとスライスを保存するための変数
	ID3D12Resource* barrierResource;
	uint32_t barrierSlice;
	uint32_t targetDSVIndex;
	Matrix4x4 lVPM_;

	//スポットライト専用変数
	bool isSLFinish = false;
	int slSliceIndex = -1;

	
};

