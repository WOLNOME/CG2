#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <array>

//モデル共通部
class Object3dCommon
{
public:
	enum class NameGPS {
		None,			//通常
		Animation,		//アニメーション
		SkyBox,			//スカイボックス

		kMaxNumNameGPS,	//最大数
	};

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
	void SettingCommonDrawing(NameGPS index = NameGPS::None);
	//アニメーション専用コンピュートシェーダー前設定
	void SettingAnimationCS();

private://非公開メンバ関数
	//グラフィックスパイプライン
	void GenerateGraphicsPipeline();
	//通常のPSO設定
	void NormalPSOOption();
	//アニメーション用のPSO設定
	void AnimationPSOOption();
	//スカイボックス用のPSO設定
	void SkyBoxPSOOption();

private://メンバ変数
	//ルートシグネチャ
	std::array<Microsoft::WRL::ComPtr<ID3D12RootSignature>, (int)NameGPS::kMaxNumNameGPS> rootSignature;
	//グラフィックスパイプライン
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, (int)NameGPS::kMaxNumNameGPS> graphicsPipelineState;

	//コンピュートルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature = nullptr;
	//コンピュートパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> computePipelineState = nullptr;

};

