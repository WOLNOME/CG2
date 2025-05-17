#include "MainRender.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "GPUDescriptorManager.h"
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
	//各種デスクリプターヒープの生成
	GenerateDescriptorHeap();
	//レンダーターゲットビューの初期化
	InitRenderTargetView();
	//深度ステンシルビューの初期化
	InitDepthStencilView();
	//ビューポート矩形の初期化
	InitViewPort();
	//シザー矩形の初期化
	InitScissorRect();

	//オフスク用レンダーテクスチャの生成
	InitOffScreenRenderingOption();
	//オフスク用グラフィックスパイプラインの生成
	GenerateRenderTextureGraphicsPipeline();
}

void MainRender::Finalize() {
	delete instance;
	instance = nullptr;
}

void MainRender::PreObjectDraw() {
	///--------------------------------------------///
	/// RenderTextureに対する設定
	///--------------------------------------------///

	//これから書き込むRenderTextureのIndexを取得
	UINT renderTextureIndex = 2;

	//描画先のRTVとDSVを設定するを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DirectXCommon::GetCPUDescriptorHandle(dsvDescriptorHeap.Get(), descriptorSizeDSV, 0);
	commandList->OMSetRenderTargets(1, &rtvHandles[renderTextureIndex], false, &dsvHandle);
	//クリアバリューの色で画面全体をクリアする
	float clearColor[] = {
		DirectXCommon::GetInstance()->GetClearValue().Color[0],
		DirectXCommon::GetInstance()->GetClearValue().Color[1],
		DirectXCommon::GetInstance()->GetClearValue().Color[2],
		DirectXCommon::GetInstance()->GetClearValue().Color[3]
	};
	commandList->ClearRenderTargetView(rtvHandles[renderTextureIndex], clearColor, 0, nullptr);
	//指定した深度で画面全体をクリアする
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//コマンドを積む
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
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	//指定した色で画面全体をクリアする
	float clearColor[] = { 0,1,0,0 };//青っぽい色。RGBAの順
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	//※深度バッファはクリアしない

	//コマンドを積む
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	//バリアの設定
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTextureResource.Get();		//レンダーテクスチャに対して行う
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;			//遷移前の状態
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;		//遷移後の状態
	commandList->ResourceBarrier(1, &barrier);

	//描画情報コピー
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(graphicsPipelineState.Get());
	commandList->SetGraphicsRootDescriptorTable(0, GPUDescriptorManager::GetInstance()->GetGPUDescriptorHandle(rtIndex));
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);

	//バリアの設定
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTextureResource.Get();		//レンダーテクスチャに対して行う
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;	//遷移前の状態
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;				//遷移後の状態
	commandList->ResourceBarrier(1, &barrier);

}

void MainRender::PostDraw() {
	HRESULT hr;
	//!>本来ここにバリア遷移があるけど、D2Dの描画終了時に同じことをしているので省略

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

void MainRender::GenerateDescriptorHeap() {
	//サイズ
	descriptorSizeRTV = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//RTV用のヒープディスクリプタの数は3(ダブルバッファリング+オフスク)。RTVはShader内で触るものではないので、ShaderVisibleはfalse
	rtvDescriptorHeap = DirectXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, false);
	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものなのではないので、ShaderVisbleはfalse
	dsvDescriptorHeap = DirectXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
}

void MainRender::InitRenderTargetView() {
	HRESULT hr;
	//SwapChainからResourceを引っ張ってくる
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//上手く取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

	//RTVの設定
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; //2dのテクスチャとして書き込む
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvStartHandle = DirectXCommon::GetCPUDescriptorHandle(rtvDescriptorHeap.Get(), descriptorSizeRTV, 0);
	//まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;
	DirectXCommon::GetInstance()->GetDevice()->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles[1].ptr = rtvHandles[0].ptr + DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2つ目を作る
	DirectXCommon::GetInstance()->GetDevice()->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);
}

void MainRender::InitDepthStencilView() {
	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//DSVHeapの先頭にDSVを作る
	DirectXCommon::GetInstance()->GetDevice()->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, DirectXCommon::GetCPUDescriptorHandle(dsvDescriptorHeap.Get(), descriptorSizeDSV, 0));
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

void MainRender::InitOffScreenRenderingOption() {
	//RTVディスクリプタハンドルの取得
	UINT rtvDescriptorSize = DirectXCommon::GetInstance()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	rtvHandles[2].ptr = rtvHandles[1].ptr + rtvDescriptorSize;
	//RTVの作成
	const Vector4 kRenderTragetClearValue = Vector4(1, 1, 0, 0);
	renderTextureResource = DirectXCommon::GetInstance()->CreateRenderTextureResource(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTragetClearValue);
	DirectXCommon::GetInstance()->GetDevice()->CreateRenderTargetView(renderTextureResource.Get(), &rtvDesc, rtvHandles[2]);
	//SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC renderTextureSrvDesc{};
	renderTextureSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	renderTextureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	renderTextureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	renderTextureSrvDesc.Texture2D.MipLevels = 1;
	rtIndex = GPUDescriptorManager::GetInstance()->Allocate();
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(renderTextureResource.Get(), &renderTextureSrvDesc, GPUDescriptorManager::GetInstance()->GetCPUDescriptorHandle(rtIndex));
}

void MainRender::GenerateRenderTextureGraphicsPipeline() {
	HRESULT hr;

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[1] = {};
	//テクスチャの設定
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Tableを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//Signatureに反映
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3D10Blob> signatireBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatireBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとに生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatireBlob->GetBufferPointer(),
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//InputLayoutは利用しない
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = nullptr;
	inputLayoutDesc.NumElements = 0;

	//ブレンドの設定
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE; // アルファ値を考慮する
	blendDesc.IndependentBlendEnable = FALSE;
	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc{};
	rtBlendDesc.BlendEnable = TRUE; // ブレンド有効
	rtBlendDesc.LogicOpEnable = FALSE;
	rtBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rtBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0] = rtBlendDesc;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面を表示する(モデルとは無関係)
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/CopyImage.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Vignette.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を無効化する
	depthStencilDesc.DepthEnable = false;

	//グラフィックスパイプラインステートに設定を反映
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジのタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//実際に生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}

D3D12_CPU_DESCRIPTOR_HANDLE MainRender::GetRTVCPUDescriptorHandle(uint32_t index) {
	return DirectXCommon::GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE MainRender::GetRTVGPUDescriptorHandle(uint32_t index) {
	return DirectXCommon::GetGPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE MainRender::GetDSVCPUDescriptorHandle(uint32_t index) {
	return DirectXCommon::GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE MainRender::GetDSVGPUDescriptorHandle(uint32_t index) {
	return DirectXCommon::GetGPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index);
}
