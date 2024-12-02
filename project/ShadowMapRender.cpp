#include "ShadowMapRender.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "SceneLight.h"
#include <cassert>

ShadowMapRender* ShadowMapRender::instance = nullptr;

using namespace Microsoft::WRL;

ShadowMapRender* ShadowMapRender::GetInstance()
{
	if (instance == nullptr) {
		instance = new ShadowMapRender;
	}
	return instance;
}

void ShadowMapRender::Initialize()
{
	//コマンド関連の初期化
	InitCommand();
	//各種デスクリプターヒープの生成
	GenerateDescriptorHeap();
}

void ShadowMapRender::Finalize()
{
	delete instance;
	instance = nullptr;
}

void ShadowMapRender::AllPostDraw()
{
	//コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
	HRESULT hr = commandList->Close();
	assert(SUCCEEDED(hr));

	//GPUにコマンドリストの実行を行わせる
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList.Get() };
	DirectXCommon::GetInstance()->GetCommandQueue()->ExecuteCommandLists(1, commandLists->GetAddressOf());

}

void ShadowMapRender::ReadyNextCommand()
{
	//次のフレーム用のコマンドリストを準備
	HRESULT hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void ShadowMapRender::InitCommand()
{
	HRESULT hr;
	//コマンドアロケーターを生成する
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケーターの生成が上手くいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	//コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
}

void ShadowMapRender::GenerateDescriptorHeap()
{
	//サイズ
	descriptorSizeDSV = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//DSV用のヒープでディスクリプタの数はSMの合計分。DSVはShader内で触るものなのではないので、ShaderVisbleはfalse
	int numDLSM = kCascadeCount * kMaxNumDirectionalLight;
	int numPLSM = 0;
	int numSLSM = 0;
	dsvDescriptorHeap = DirectXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, (numDLSM + numPLSM + numSLSM), false);
}

D3D12_CPU_DESCRIPTOR_HANDLE ShadowMapRender::GetDSVCPUDescriptorHandle(uint32_t index)
{
	return DirectXCommon::GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE ShadowMapRender::GetDSVGPUDescriptorHandle(uint32_t index)
{
	return DirectXCommon::GetGPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}
