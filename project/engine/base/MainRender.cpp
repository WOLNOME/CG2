#include "MainRender.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "GPUDescriptorManager.h"
#include "RTVManager.h"
#include "DSVManager.h"
#include "Logger.h"
#include <cassert>

MainRender* MainRender::instance = nullptr;

using namespace Microsoft::WRL;

MainRender* MainRender::GetInstance() {
	if (instance == nullptr) {
		instance = new MainRender;
	}
	return instance;
}

void MainRender::Initialize() {
	//コマンド関連の初期化
	InitCommand();
	//スワップチェーンの生成
	GenerateSwapChain();
	//深度バッファの生成
	GenerateDepthBuffer();
	//レンダーターゲットビューの初期化
	InitRenderTargetView();
	//デプスステンシルビューの初期化
	InitDepthStencilView();
	//ビューポート矩形の初期化
	InitViewPort();
	//シザー矩形の初期化
	InitScissorRect();
}

void MainRender::Finalize() {
	delete instance;
	instance = nullptr;
}

void MainRender::PreObjectDraw() {
	//ビューポートとシザー矩形を設定する
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

}

void MainRender::PreImGuiDraw() {
	///--------------------------------------------///
	/// SwapChainに対する設定
	///--------------------------------------------///

	//これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//バリアの設定
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();	//スワップチェインのバックバッファに対して行う
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;				//遷移前の状態
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;			//遷移後の状態
	commandList->ResourceBarrier(1, &barrier);

	//描画先のRTVを設定する(深度バッファは送らない)
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RTVManager::GetInstance()->GetCPUDescriptorHandle(rtvIndices[backBufferIndex]);
	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
	//指定した色で画面全体をクリアする
	float clearColor[] = { 0,1,0,0 };//青っぽい色。RGBAの順
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	//※深度バッファはクリアしない

	//コマンドを積む
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}

void MainRender::PostDraw() {
	HRESULT hr;
	//!>本来ここにスワップチェーンリソースバリア遷移があるけど、D2Dの描画終了時に同じことをしているので省略

	//コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	//GPUにコマンドリストの実行を行わせる
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList.Get() };
	DirectXCommon::GetInstance()->GetCommandQueue()->ExecuteCommandLists(1, commandLists->GetAddressOf());
}

void MainRender::ExchangeScreen() {
	//GPUとOSに画面の交換を行うように通知する
	swapChain->Present(1, 0);
}

void MainRender::ReadyNextCommand() {
	//次のフレーム用のコマンドリストを準備
	HRESULT hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void MainRender::InitCommand() {
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

void MainRender::GenerateSwapChain() {
	HRESULT hr;
	//デスク
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//スワップチェーンを生成設定
	swapChainDesc.Width = WinApp::kClientWidth;	//画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = WinApp::kClientHeight; //画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //色の領域
	swapChainDesc.SampleDesc.Count = 1; //マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2; //ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //モニタにうつしたら、中身を破棄
	//スワップチェーンを生成する。
	hr = DirectXCommon::GetInstance()->GetDXGIFactory()->CreateSwapChainForHwnd(DirectXCommon::GetInstance()->GetCommandQueue(), WinApp::GetInstance()->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

void MainRender::GenerateDepthBuffer() {
	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = WinApp::kClientWidth;
	resourceDesc.Height = WinApp::kClientHeight;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//利用するヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));

	//リソース生成
	depthStencilResource = resource;
}

void MainRender::InitRenderTargetView() {
	HRESULT hr;
	//SwapChainからResourceを引っ張ってくる
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//上手く取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	//スワップチェーン用のRTVを作成する
	rtvIndices[0] = RTVManager::GetInstance()->Allocate();
	rtvIndices[1] = RTVManager::GetInstance()->Allocate();
	RTVManager::GetInstance()->CreateRTVDescriptor(rtvIndices[0], swapChainResources[0].Get());
	RTVManager::GetInstance()->CreateRTVDescriptor(rtvIndices[1], swapChainResources[1].Get());

}

void MainRender::InitDepthStencilView() {
	//深度バッファのDSVを作成する
	dsvIndex = DSVManager::GetInstance()->Allocate();
	DSVManager::GetInstance()->CreateDSVDescriptor(dsvIndex, depthStencilResource.Get());
}

void MainRender::InitViewPort() {
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void MainRender::InitScissorRect() {
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;
}
