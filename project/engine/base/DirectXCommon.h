#pragma once
#include "Vector4.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>
#include <chrono>

//DirectXTex
#include "DirectXTex.h"

#pragma comment(lib,"dxcompiler.lib")

class DirectXCommon {
private://コンストラクタ等の隠蔽
	static DirectXCommon* instance;

	DirectXCommon() = default;//コンストラクタ隠蔽
	~DirectXCommon() = default;//デストラクタ隠蔽
	DirectXCommon(DirectXCommon&) = delete;//コピーコンストラクタ封印
	DirectXCommon& operator=(DirectXCommon&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static DirectXCommon* GetInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();

	//各レンダーごとの後処理
	void PostEachRender();
	//全てのレンダーが完了時の後処理
	void PostAllRenders();

private://生成系メンバ関数
	void GenerateDevice();//共通
	void InitCommand();//コマンドキューのみ共通
	void GenerateFence();//共通
	void GenerateDXCCompiler();//共通

private:
	//FPS固定初期化
	void InitializeFixFPS();
	//FPS固定更新
	void UpdateFixFPS();
	//記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;

public://公開メンバ関数
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	//コンパイルシェーダー
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		//CompilerするShaderファイルへのパス
		const std::wstring& filePath,
		//Compilerに使用するProfile
		const wchar_t* profile
	);
	//デスクリプタヒープ作成
	ID3D12DescriptorHeap* CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	//リソース生成
	ID3D12Resource* CreateBufferResource(size_t sizeInBytes);
	//UAV用リソース作成
	ID3D12Resource* CreateUAVBufferResource(size_t sizeInBytes);
	//テクスチャリソースの生成
	ID3D12Resource* CreateTextureResource(const DirectX::TexMetadata& metadata);
	//レンダーテクスチャリソースの生成
	ID3D12Resource* CreateRenderTextureResource(uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor, bool isSharedD2D = false);

public://getter
	//デバイス
	ID3D12Device* GetDevice() const { return device.Get(); }
	//DXGIファクトリ
	IDXGIFactory7* GetDXGIFactory() const { return dxgiFactory.Get(); }
	//コマンドキュー
	ID3D12CommandQueue* GetCommandQueue() const { return commandQueue.Get(); }
	ID3D12CommandQueue** GetAddressOfCommandQueue() { return commandQueue.GetAddressOf(); }

private://メンバ変数
	//DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	//コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	//フェンス
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	//フェンス値
	uint64_t fenceValue = 0;
	//イベント
	HANDLE fenceEvent;
	//DXCユーティリティ
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	//DXCコンパイラ
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	//インクルードハンドラ
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;

};

