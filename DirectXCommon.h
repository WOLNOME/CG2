#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <array>
#include <dxcapi.h>
#include "WinApp.h"

//imgui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#pragma comment(lib,"dxcompiler.lib")

class DirectXCommon
{
public:
	void Initialize(WinApp* winApp);

private://生成系メンバ変数
	void GenerateDevice();
	void InitCommand();
	void GenerateSwapChain();
	void GenerateDepthBuffer();
	void GenerateDescriptorHeap();
	void InitRenderTargetView();
	void InitDepthStencilView();
	void GenerateFence();
	void InitViewPort();
	void InitScissorRect();
	void GenerateDXCCompiler();
	void InitImGui();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

public:
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);

private://インスタンス
	//WindowsAPI
	WinApp* winApp_ = nullptr;
private://メンバ変数
	//DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	//コマンドアロケーター
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	//スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	//スワップチェーンデスク
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//DepthStencilResource
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;
	//デスクリプターヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;
	//デスクリプターサイズ
	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//スワップチェーンから引っ張て来たリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
	//RTVハンドル
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtvHandles;
	//RTVデスク
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	//イベント
	HANDLE fenceEvent;
	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};
	//DXCユーティリティ
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	//DXCコンパイラ
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	//インクルードハンドラ
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;


};

