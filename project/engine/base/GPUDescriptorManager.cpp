#include "GPUDescriptorManager.h"
#include "DirectXCommon.h"
#include <cassert>

GPUDescriptorManager* GPUDescriptorManager::instance = nullptr;
const uint32_t GPUDescriptorManager::kMaxHeapSize = 512;		//必要に応じて増やす

GPUDescriptorManager* GPUDescriptorManager::GetInstance() {
	if (instance == nullptr) {
		instance = new GPUDescriptorManager;
	}
	return instance;
}

void GPUDescriptorManager::Initialize() {
	descriptorHeap = DirectXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxHeapSize, true);
	descriptorSize = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void GPUDescriptorManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void GPUDescriptorManager::PreDraw(ID3D12GraphicsCommandList* pCommandList) {
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	pCommandList->SetDescriptorHeaps(1, descriptorHeaps);
}

uint32_t GPUDescriptorManager::Allocate() {
	// 空きインデックスがあれば再利用
	if (!enableIndices.empty()) {
		uint32_t index = enableIndices.front();
		enableIndices.pop();
		return index;
	}

	// 上限に達していないかチェック
	if (useIndex >= kMaxHeapSize) {
		assert(0 && "ヒープの中身が上限に達しました");
		return UINT32_MAX; // エラーの場合
	}

	// 新しいインデックスを割り当てる
	return useIndex++;
}

void GPUDescriptorManager::Free(uint32_t index) {
	// インデックスが範囲内であることを確認
	if (index < kMaxHeapSize) {
		unenableIndices.push(index);
	}
}

bool GPUDescriptorManager::CheckCanSecured() {
	return (useIndex < kMaxHeapSize || !enableIndices.empty());
}

void GPUDescriptorManager::TransferEnable() {
	while (!unenableIndices.empty()) {
		// フレームフリーキューの先頭から1つ取り出して
		uint32_t index = unenableIndices.front();
		unenableIndices.pop();
		// フリーキューに追加
		enableIndices.push(index);
	}
}

void GPUDescriptorManager::CreateSRVforRenderTexture(uint32_t index, ID3D12Resource* pResource) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(index));
}

void GPUDescriptorManager::CreateSRVforStructuredBuffer(uint32_t index, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = structureByteStride;
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(index));
}

void GPUDescriptorManager::CreateUAVforRWStructuredBuffer(uint32_t index, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride) {
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = numElements;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.StructureByteStride = structureByteStride;
	DirectXCommon::GetInstance()->GetDevice()->CreateUnorderedAccessView(pResource, nullptr, &uavDesc, GetCPUDescriptorHandle(index));
}

D3D12_CPU_DESCRIPTOR_HANDLE GPUDescriptorManager::GetCPUDescriptorHandle(uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorManager::GetGPUDescriptorHandle(uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

void GPUDescriptorManager::SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* pCommandList, UINT RootParameterIndex, uint32_t index) {
	pCommandList->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(index));
}
