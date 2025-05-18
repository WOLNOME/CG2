#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <array>

//ポストエフェクトの種類
enum class PostEffectKind {
	None,					// 何もしない
	Grayscale,				// グレースケール
	Vignette,				// ヴィネット	
	BoxFilter,				// ボックスフィルター
	GaussianFilter,			// ガウシアンフィルター
	LuminanceBaseOutline,	// 輝度ベースのアウトライン
	RadialBlur,				// ラジアルブラー

	kMaxNumPostEffectKind,	// ポストエフェクトの最大数
};
//もしポストエフェクトを追加した場合
//ルートシグネチャを追加したい場合は別途設定必須
//グラフィックスパイプラインではPSを増やす
//ImGuiにも追加しておく

class PostEffectManager {
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

	//現在適用しているポストエフェクトの種類
	PostEffectKind currentPostEffectKind = PostEffectKind::None;
};

