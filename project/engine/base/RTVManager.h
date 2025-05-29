#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <queue>

class RTVManager {
private:
	static RTVManager* instance;

	RTVManager() = default;
	~RTVManager() = default;
	RTVManager(RTVManager&) = delete;
	RTVManager& operator=(RTVManager&) = delete;

public:
	static RTVManager* GetInstance();

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
	// 使用不可能インデックスを使用可能インデックスに遷移させる関数
	void TransferEnable();

	// RTVDescriptor生成関数
	void CreateRTVDescriptor(uint32_t index, ID3D12Resource* pResource);
	
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

	// 使用可能空きインデックスを管理するキュー
	std::queue<uint32_t> enableIndices;
	// 使用不可能空きインデックスを管理するキュー
	std::queue<uint32_t> unenableIndices;

};

