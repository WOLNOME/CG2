#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>

class ShadowMapRender
{
private://コンストラクタ等の隠蔽
	static ShadowMapRender* instance;

	ShadowMapRender() = default;//コンストラクタ隠蔽
	~ShadowMapRender() = default;//デストラクタ隠蔽
	ShadowMapRender(ShadowMapRender&) = delete;//コピーコンストラクタ封印
	ShadowMapRender& operator=(ShadowMapRender&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static ShadowMapRender* GetInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();

	//全ての描画後処理
	void AllPostDraw();
	//次のコマンドの準備
	void ReadyNextCommand();

private://生成系メンバ関数
	void InitCommand();
	void GenerateDescriptorHeap();

public://公開メンバ関数
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);
public://公開メンバ変数

public://ゲッター
	//コマンドリスト
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
	
private://インスタンス

private://メンバ変数
	//コマンドアロケーター
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//DSVデスクリプターヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;
	//DSVデスクリプターサイズ
	uint32_t descriptorSizeDSV = 0;
	
};

