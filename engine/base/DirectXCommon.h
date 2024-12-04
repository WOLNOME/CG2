#pragma once
#include <windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <array>
#include <dxcapi.h>
#include <string>
#include <chrono>

//imgui
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
//DirectXTex
#include "DirectXTex.h"

#pragma comment(lib,"dxcompiler.lib")

class DirectXCommon
{
private://コンストラクタ等の隠蔽
	static DirectXCommon* instance;

	DirectXCommon() = default;//コンストラクタ隠蔽
	~DirectXCommon() = default;//デストラクタ隠蔽
	DirectXCommon(DirectXCommon&) = delete;//コピーコンストラクタ封印
	DirectXCommon& operator=(DirectXCommon&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static DirectXCommon* GetInstance();
	//インスタンスの破棄
	void DeleteInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();


	//描画前処理
	void PreDraw();
	//描画後処理
	void PostDraw();

private://生成系メンバ関数
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
	
private:
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	//FPS固定初期化
	void InitializeFixFPS();
	//FPS固定更新
	void UpdateFixFPS();
	//記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;

public://公開メンバ関数
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);
	//コンパイルシェーダー
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		//CompilerするShaderファイルへのパス
		const std::wstring& filePath,
		//Compilerに使用するProfile
		const wchar_t* profile
	);
	//デスクリプタヒープ作成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	//リソース生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
	//テクスチャリソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
	
	
public://公開メンバ変数
	
public://ゲッター
	//デバイス
	ID3D12Device* GetDevice() const { return device.Get(); }
	//コマンドリスト
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
	//バックバッファの数を取得
	size_t GetBackBufferCount()const { return swapChainDesc.BufferCount; }

private://インスタンス

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
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;
	//デスクリプターサイズ
	uint32_t descriptorSizeRTV = 0;
	uint32_t descriptorSizeDSV = 0;
	//スワップチェーンから引っ張て来たリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
	//RTVハンドル
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtvHandles;
	//RTVデスク
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	//フェンス値
	uint64_t fenceValue = 0;
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
	//TransitionBarrier
	D3D12_RESOURCE_BARRIER barrier{};
	
};

