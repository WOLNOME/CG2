#include "TextTextureRender.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "GPUDescriptorManager.h"
#include "RTVManager.h"
#include <cassert>

TextTextureRender* TextTextureRender::instance = nullptr;

using namespace Microsoft::WRL;

TextTextureRender* TextTextureRender::GetInstance() {
	if (instance == nullptr) {
		instance = new TextTextureRender;
	}
	return instance;
}

void TextTextureRender::Initialize() {
	//コマンドリストの生成
	InitCommand();
	//ビューポートの初期化
	InitViewPort();
	//シザーの初期化
	InitScissorRect();
}

void TextTextureRender::Finalize() {
	delete instance;
	instance = nullptr;
}

void TextTextureRender::PreDraw() {
	//ビューポートとシザー矩形を設定する
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}

void TextTextureRender::PostDraw() {
	HRESULT hr;
	//コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	//GPUにコマンドリストの実行を行わせる
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { commandList.Get() };
	DirectXCommon::GetInstance()->GetCommandQueue()->ExecuteCommandLists(1, commandLists->GetAddressOf());
}

void TextTextureRender::ReadyNextCommand() {
	HRESULT hr;
	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

void TextTextureRender::InitCommand() {
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

void TextTextureRender::InitViewPort() {
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void TextTextureRender::InitScissorRect() {
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;
}
