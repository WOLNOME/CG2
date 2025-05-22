#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <queue>

class DSVManager {
private:
	static DSVManager* instance;

	DSVManager() = default;
	~DSVManager() = default;
	DSVManager(DSVManager&) = delete;
	DSVManager& operator=(DSVManager&) = delete;

public:
	static DSVManager* GetInstance();

	// 初期化
	void Initialize();
	// 終了
	void Finalize();

	// 割り当て用関数
	uint32_t Allocate();
	// 解放用関数
	void Free(uint32_t index);
	// 空きインデックスの存在確認用関数
	bool CheckCanSecured();

	// DSVDescriptor生成関数
	void CreateDSVDescriptor(uint32_t index, ID3D12Resource* pResource);

	// ゲッター
	ID3D12DescriptorHeap* GetDescriptorHeap() const { return descriptorHeap.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	static const uint32_t kMaxHeapSize;

private:
	//デスクリプタのサイズ
	uint32_t descriptorSize = 0;
	//デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	//最新の空きインデックス
	uint32_t useIndex = 0;

	// 空きインデックスを管理するキュー(Free関数によって割り当てられる)
	std::queue<uint32_t> freeIndices;
};

