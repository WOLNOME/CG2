#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>

class DirectXCommon;
class SrvManager
{
public:
	//初期化
	void Initialize(DirectXCommon* dxCommon);


	void PreDraw();
	uint32_t Allocate();
	bool CheckCanSecured();

	//SRV生成(テクスチャ用)
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);
	//SRV生成(Structured Buffer用)
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);


public://ゲッター
	ID3D12DescriptorHeap* GetDescriptorHeap()const { return descriptorHeap.Get(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);
public://セッター
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

private:

public://公開メンバ変数
	//最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;

private://インスタンス
	DirectXCommon* dxCommon_ = nullptr;
private://メンバ変数
	//SRV用のデスクリプタサイズ
	uint32_t descriptorSize = 0;
	//SRV用デスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;

	//次に使用するSRVインデックス
	uint32_t useIndex = 0;


};
