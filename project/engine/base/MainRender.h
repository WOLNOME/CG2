#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>
#include <array>

class MainRender
{
private://コンストラクタ等の隠蔽
	static MainRender* instance;

	MainRender() = default;//コンストラクタ隠蔽
	~MainRender() = default;//デストラクタ隠蔽
	MainRender(MainRender&) = delete;//コピーコンストラクタ封印
	MainRender& operator=(MainRender&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static MainRender* GetInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();

	//オブジェクト描画前処理
	void PreObjectDraw();
	//ImGui描画前処理
	void PreImGuiDraw();
	//描画後処理
	void PostDraw();
	//画面切り替え処理
	void ExchangeScreen();

	//コマンドの準備
	void ReadyNextCommand();
private://生成系メンバ関数
	void InitCommand();
	void GenerateSwapChain();
	void GenerateDepthBuffer();
	void GenerateDescriptorHeap();
	void InitRenderTargetView();
	void InitDepthStencilView();
	void InitViewPort();
	void InitScissorRect();

public://公開メンバ関数
	//DSVデスクリプタヒープの取得
	ID3D12DescriptorHeap* GetDSVDescriptorHeap()const { return dsvDescriptorHeap.Get(); }
	//DSVデスクリプタのサイズ取得
	uint32_t GetDSVDescriptorSize()const { return descriptorSizeDSV; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);
public://公開メンバ変数

public://ゲッター
	//コマンドアロケーター
	ID3D12CommandAllocator* GetCommandAllocator() const { return commandAllocator.Get(); }
	//コマンドリスト
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
	//バックバッファの数を取得
	size_t GetBackBufferCount()const { return swapChainDesc.BufferCount; }
	//スワップチェーン
	IDXGISwapChain4* GetSwapChain()const { return swapChain.Get(); }
	//スワップチェーンのリソース
	ID3D12Resource* GetSwapChainResource(uint32_t index)const { return swapChainResources[index].Get(); }
	//RTV

private://メンバ変数
	//コマンドアロケーター
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	//スワップチェーンデスク
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//DepthStencilResource
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;
	//デスクリプターヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;
	//デスクリプターサイズ
	uint32_t descriptorSizeDSV = 0;
	//スワップチェーンから引っ張て来たリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
	//RTVインデックス
	std::array<uint32_t, 2> rtvHandles;
	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};

};

