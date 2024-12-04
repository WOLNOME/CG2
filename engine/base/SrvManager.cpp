#include "SrvManager.h"
#include "DirectXCommon.h"
#include <cassert>

SrvManager* SrvManager::instance = nullptr;

const uint32_t SrvManager::kMaxSRVCount = 512;

SrvManager* SrvManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new SrvManager;
	}
	return instance;
}

void SrvManager::Initialize()
{
	//デスクリプタヒープの生成
	descriptorHeap = DirectXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	//デスクリプタの1個分のサイズを取得して記録
	descriptorSize = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void SrvManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void SrvManager::PreDraw()
{
	//描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	DirectXCommon::GetInstance()->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = MipLevels;

	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetCPUDescriptorHandle(srvIndex));
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride)
{

}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex)
{
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}

uint32_t SrvManager::Allocate()
{
	//上限に達していないかチェック
	if (useIndex > kMaxSRVCount) {
		assert(0);
	}

	//returnする番号を一旦記録しておく
	int index = useIndex;
	//次回のために番号を一旦記録しておく
	useIndex++;
	//上で記録した番号をreturn
	return index;

}

bool SrvManager::CheckCanSecured()
{
	//最大テクスチャ枚数を超えて読み込もうとしたらfalse
	if (useIndex > kMaxSRVCount) {
		return false;
	}
	return true;
}
