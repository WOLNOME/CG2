#include "DSVManager.h"
#include "DirectXCommon.h"
#include <cassert>

DSVManager* DSVManager::instance = nullptr;
const uint32_t DSVManager::kMaxHeapSize = 16;		//必要に応じて増やす

DSVManager* DSVManager::GetInstance() {
    if (instance == nullptr) {
        instance = new DSVManager;
    }
    return instance;
}

void DSVManager::Initialize() {
    //サイズ
    descriptorSize = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    //DSV用のヒープ。DSVはShader内で触るものなのではないので、ShaderVisbleはfalse
    descriptorHeap = DirectXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, kMaxHeapSize, false);
}

void DSVManager::Finalize() {
    delete instance;
    instance = nullptr;
}

uint32_t DSVManager::Allocate() {
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

void DSVManager::Free(uint32_t index) {
    // インデックスが範囲内であることを確認
    if (index < kMaxHeapSize) {
        unenableIndices.push(index);
    }
}

bool DSVManager::CheckCanSecured() {
    return (useIndex < kMaxHeapSize || !enableIndices.empty());
}

void DSVManager::TransferEnable() {
    while (!unenableIndices.empty()) {
        // フレームフリーキューの先頭から1つ取り出して
        uint32_t index = unenableIndices.front();
        unenableIndices.pop();
        // フリーキューに追加
        enableIndices.push(index);
    }
}

void DSVManager::CreateDSVDescriptor(uint32_t index, ID3D12Resource* pResource) {
    //DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    //DSVHeapの先頭にDSVを作る
    DirectXCommon::GetInstance()->GetDevice()->CreateDepthStencilView(pResource, &dsvDesc, DirectXCommon::GetCPUDescriptorHandle(descriptorHeap.Get(), descriptorSize, index));
}

D3D12_CPU_DESCRIPTOR_HANDLE DSVManager::GetCPUDescriptorHandle(uint32_t index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += descriptorSize * index;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DSVManager::GetGPUDescriptorHandle(uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += descriptorSize * index;
    return handle;
}
