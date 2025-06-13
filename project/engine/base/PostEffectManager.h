#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <array>
#include <vector>
#include "Vector3.h"
#include "Vector4.h"

//ポストエフェクトの種類
enum class PostEffectKind {
	None,					// 何もしない
	Grayscale,				// グレースケール
	Vignette,				// ヴィネット	
	BoxFilter,				// ボックスフィルター
	GaussianFilter,			// ガウシアンフィルター
	LuminanceBaseOutline,	// 輝度ベースのアウトライン
	RadialBlur,				// ラジアルブラー
	Dissolve,				// ディゾルブ
	Random,					// ランダム
	HSVFilter,				// HSVフィルター

	kMaxNumPostEffectKind,	// ポストエフェクトの最大数
};
//もしポストエフェクトを追加した場合
//ルートシグネチャを追加したい場合は別途設定必須
//グラフィックスパイプラインではPSを増やす
//ImGuiにも追加しておく
//描画の個別設定も別途必要

class PostEffectManager {
private://構造体
	//ディゾルブ系
	struct DissolveData {
		float threshold;	//閾値

		//追加予定項目
		//全体の色、エッジの色、エッジの大きさ
	};
	struct DissolveResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		DissolveData* data;
		uint32_t textureHandle;		//ディゾルブに使用するテクスチャ
	};
	//ランダム系
	struct RandomData {
		float seed;	//シード値
	};
	struct RandomResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		RandomData* data;
	};
	//HSVフィルター系
	struct HSVFilterData {
		Vector3 hsvColor;	//HSVの色
	};
	struct HSVFilterResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		HSVFilterData* data;
	};
	
	//全ポストエフェクトのリソース管理用構造体
	struct PostEffectResource {
		DissolveResource dissolveResource;
		RandomResource randomResource;
		HSVFilterResource hsvResource;
	};

private://コンストラクタ等の隠蔽
	static PostEffectManager* instance;

	PostEffectManager() = default;//コンストラクタ隠蔽
	~PostEffectManager() = default;//デストラクタ隠蔽
	PostEffectManager(PostEffectManager&) = delete;//コピーコンストラクタ封印
	PostEffectManager& operator=(PostEffectManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static PostEffectManager* GetInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();

	//オブジェクト描画前処理
	void PreObjectDraw();

	//シーンのコピー
	void CopySceneToRenderTexture();

	//デバッグ用ImGui
	void DebugWithImGui();

private://生成系メンバ関数

	//オフスクの初期化
	void InitOffScreenRenderingOption();
	//オフスクのグラフィックスパイプラインの生成
	void GenerateRenderTextureGraphicsPipeline();
	//固有リソースの初期化
	void InitUniqueResources();

private:
	//レンダーテクスチャのリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource = nullptr;
	//レンダーテクスチャのSRVインデックス
	uint32_t srvIndex = 0;
	//ルートシグネチャ
	std::array<Microsoft::WRL::ComPtr<ID3D12RootSignature>,(int)PostEffectKind::kMaxNumPostEffectKind> rootSignature;
	//グラフィックスパイプライン
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, (int)PostEffectKind::kMaxNumPostEffectKind> graphicsPipelineState;
	//RTVのディスクリプタハンドル
	uint32_t rtvIndex = 0;
	//レンダーテクスチャのクリアカラー
	const Vector4 kRenderTragetClearValue = Vector4(0, 0, 1, 1);

	//現在適用しているポストエフェクトの種類
	PostEffectKind currentPostEffectKind = PostEffectKind::None;

	//ポストエフェクトのリソース
	PostEffectResource postEffectResource;
};

