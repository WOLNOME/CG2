#include "RTVManager.h"
#include "DirectXCommon.h"
#include <cassert>

RTVManager* RTVManager::instance = nullptr;
const uint32_t RTVManager::kMaxHeapSize = 256;		//必要に応じて増やす

RTVManager* RTVManager::GetInstance() {
    if (instance == nullptr) {
        instance = new RTVManager;
    }
    return instance;
}

void RTVManager::Initialize() {
    //RTV用のヒープディスクリプタ。RTVはShader内で触るものではないので、ShaderVisibleはfalse
    descriptorHeap = DirectXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kMaxHeapSize, false);
    //デスクリプターサイズ
    descriptorSize = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void RTVManager::Finalize() {
    delete instance;
    instance = nullptr;
}

uint32_t RTVManager::Allocate() {
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

void RTVManager::Free(uint32_t index) {
	// インデックスが範囲内であることを確認
	if (index < kMaxHeapSize) {
		unenableIndices.push(index);
	}
}

bool RTVManager::CheckCanSecured() {
	return (useIndex < kMaxHeapSize || !enableIndices.empty());
}

void RTVManager::TransferEnable() {
	while (!unenableIndices.empty()) {
		// フレームフリーキューの先頭から1つ取り出して
		uint32_t index = unenableIndices.front();
		unenableIndices.pop();
		// フリーキューに追加
		enableIndices.push(index);
	}
}

void RTVManager::CreateRTVDescriptor(uint32_t index, ID3D12Resource* pResource) {
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; //2dのテクスチャとして書き込む
	DirectXCommon::GetInstance()->GetDevice()->CreateRenderTargetView(pResource, &rtvDesc, GetCPUDescriptorHandle(index));
}

D3D12_CPU_DESCRIPTOR_HANDLE RTVManager::GetCPUDescriptorHandle(uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += descriptorSize * index;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE RTVManager::GetGPUDescriptorHandle(uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += descriptorSize * index;
	return handle;
}
