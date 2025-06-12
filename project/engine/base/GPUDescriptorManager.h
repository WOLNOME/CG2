#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <queue>

class GPUDescriptorManager {
private:
	static GPUDescriptorManager* instance;

	GPUDescriptorManager() = default;
	~GPUDescriptorManager() = default;
	GPUDescriptorManager(GPUDescriptorManager&) = delete;
	GPUDescriptorManager& operator=(GPUDescriptorManager&) = delete;

public:
	static GPUDescriptorManager* GetInstance();

	// 初期化
	void Initialize();
	// 終了
	void Finalize();

	// 描画前設定
	void PreDraw(ID3D12GraphicsCommandList* pCommandList);
	// 割り当て用関数
	uint32_t Allocate();
	// 解放用関数
	void Free(uint32_t index);
	// 空きインデックスの存在確認用関数
	bool CheckCanSecured();
	// 使用不可能インデックスを使用可能インデックスに遷移させる関数
	void TransferEnable();

	// SRV生成関数
	void CreateSRVforRenderTexture(uint32_t index, ID3D12Resource* pResource);
	void CreateSRVforStructuredBuffer(uint32_t index, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);
	// UAV生成関数
	void CreateUAVforRWStructuredBuffer(uint32_t index, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

	// ゲッター
	ID3D12DescriptorHeap* GetDescriptorHeap() const { return descriptorHeap.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);
	void SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* pCommandList, UINT RootParameterIndex, uint32_t index);

	static const uint32_t kMaxHeapSize;

private:
	//デスクリプタのサイズ
	uint32_t descriptorSize = 0;
	//デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	//最新の空きインデックス
	uint32_t useIndex = 0;

	// 使用可能空きインデックスを管理するキュー
	std::queue<uint32_t> enableIndices;
	// 使用不可能空きインデックスを管理するキュー
	std::queue<uint32_t> unenableIndices;

};
