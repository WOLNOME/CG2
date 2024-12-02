#include "ShadowMapManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "ShadowMapRender.h"
#include "Logger.h"
#include "DirectionalLight.h"
#include "SceneLight.h"
#include "MyMath.h"
#include <cassert>

ShadowMapManager* ShadowMapManager::instance = nullptr;

ShadowMapManager* ShadowMapManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new ShadowMapManager;
	}
	return instance;
}

void ShadowMapManager::Initialize()
{
	//LVPMリソースの初期化
	InitLVPM();
	//グラフィックスパイプラインの設定
	GenerateGraphicsPipeline();
	//平行光源情報の初期化
	InitDLShadowMapInfo();

}

void ShadowMapManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void ShadowMapManager::SettingCommonDrawing()
{
	//ルートシグネチャをセットするコマンド
	ShadowMapRender::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	//グラフィックスパイプラインステートをセットするコマンド
	ShadowMapRender::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	//プリミティブトポロジーをセットするコマンド
	ShadowMapRender::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool ShadowMapManager::SelectResource(const SceneLight* sceneLight)
{
	//平行光源のリソースを順番に返す。
	if (!isDLFinish) {
		dlCascadeIndex++;
		if (dlCascadeIndex >= kCascadeCount) {
			dlCascadeIndex = 0;
			dlSliceIndex++;
			if (dlSliceIndex >= kMaxNumDirectionalLight) {
				isDLFinish = true;
			}
		}
		if (!isDLFinish) {
			//バリアのリソースとスライスを保存してreturn
			barrierResource = dlsmInfo.resource[dlCascadeIndex].Get();
			barrierSlice = dlSliceIndex;
			std::list<std::vector<uint32_t>>::iterator it = dlsmInfo.dsvIndex.begin();
			std::advance(it, dlSliceIndex);
			targetDSVIndex = it->at(dlCascadeIndex);
			resolutionWidth = kMaxWidth >> dlCascadeIndex;
			resolutionHeight = kMaxHeight >> dlCascadeIndex;
			lVPM_ = sceneLight->GetSceneLight()->directionalLights[dlSliceIndex].cascade[dlCascadeIndex].viewProjection;
			return false;
		}
	}

	//点光源のリソースを順番に返す。

	//スポットライトのリソースを順番に返す。

	//全てのシャドウマップのレンダリングが完了したので特定の変数をリセットしてから終了する
	lVPMIndex = -1;
	dlCascadeIndex = -1;
	dlSliceIndex = 0;
	isDLFinish = false;
	return true;
}

uint32_t ShadowMapManager::PreDraw()
{
	//バリアのリセット
	barrier = {};
	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。
	barrier.Transition.pResource = barrierResource;
	//リソースのスライス
	barrier.Transition.Subresource = barrierSlice;
	//遷移前（現在）のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	//背に後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	//TransitionBarrierを張る
	ShadowMapRender::GetInstance()->GetCommandList()->ResourceBarrier(1, &barrier);

	//lVPMインデックスのインクリメント
	lVPMIndex++;
	//データに保存中のlVPMを割り当てる
	lightViewProjectionDatas_[lVPMIndex]->viewProjectionMatrix = lVPM_;

	//描画先のDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = ShadowMapRender::GetInstance()->GetDSVCPUDescriptorHandle(targetDSVIndex);
	ShadowMapRender::GetInstance()->GetCommandList()->OMSetRenderTargets(0, nullptr, false, &dsvHandle);

	//指定した深度で画面全体をクリアする
	ShadowMapRender::GetInstance()->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//ビューポート
	D3D12_VIEWPORT viewport{};
	viewport.Width = static_cast<FLOAT>(resolutionWidth);
	viewport.Height = static_cast<FLOAT>(resolutionHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//シザー矩形
	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = static_cast<LONG>(resolutionWidth);
	scissorRect.top = 0;
	scissorRect.bottom = static_cast<LONG>(resolutionHeight);
	//コマンドを積む
	ShadowMapRender::GetInstance()->GetCommandList()->RSSetViewports(1, &viewport);
	ShadowMapRender::GetInstance()->GetCommandList()->RSSetScissorRects(1, &scissorRect);

	//lVPM割り当て用指標を返す
	return lVPMIndex;
}

void ShadowMapManager::PostDraw()
{
	//終わったのでWRITEからPIXEL_SHADER_RESOURCEにする
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	//TransitionBarrierを張る
	ShadowMapRender::GetInstance()->GetCommandList()->ResourceBarrier(1, &barrier);
}

void ShadowMapManager::InitDLShadowMapInfo()
{
	//カスケードの数を登録
	dlsmInfo.cascadeNum = kCascadeCount;
	//リソースの作成
	for (int i = 0; i < kCascadeCount; i++) {
		Microsoft::WRL::ComPtr<ID3D12Resource> element;
		element = MakeTexture2DArrayResource(kMaxWidth >> i, kMaxHeight >> i, kMaxNumDirectionalLight);
		dlsmInfo.resource.push_back(element);
	}
	//DSVデスクリプタヒープにリソースとDSVを紐づけて登録
	SettingDSV();
	//SRVインデックスの設定
	SettingSRV();
}

Microsoft::WRL::ComPtr<ID3D12Resource> ShadowMapManager::MakeTexture2DResource(int width, int height)
{
	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//利用するヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	//Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&depthClearValue,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create Texture2D resource");
	}

	return resource.Get();
}

Microsoft::WRL::ComPtr<ID3D12Resource> ShadowMapManager::MakeTexture2DArrayResource(int width, int height, int numElements)
{
	// 生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;                           // ミップマップレベル数（1でミップマップなし）
	resourceDesc.DepthOrArraySize = static_cast<UINT16>(numElements); // 配列の要素数
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;          // 深度フォーマット
	resourceDesc.SampleDesc.Count = 1;                   // マルチサンプリングは無効
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2Dテクスチャ配列
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // 深度ステンシルとして使用

	// 利用するヒープの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // GPU専用ヒープ

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;    // 深度値の初期化
	depthClearValue.DepthStencil.Stencil = 0;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; // resourceDesc.Format と一致

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // 初期状態は深度書き込み
		&depthClearValue,
		IID_PPV_ARGS(&resource)
	);

	// エラーチェック
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create Texture2DArray resource");
	}

	return resource;
}

void ShadowMapManager::SettingDSV()
{
	//DSVインデックスの設定
	for (int i = 0; i < kMaxNumDirectionalLight; i++) {
		//一つの平行光源
		std::vector<uint32_t> element;
		for (int j = 0; j < kCascadeCount; j++) {
			uint32_t index;
			index = dsvIndexCount;
			dsvIndexCount++;
			element.push_back(index);
		}
		dlsmInfo.dsvIndex.push_back(element);
	}

	//DSVを作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2DArray.ArraySize = kMaxNumDirectionalLight;

	//DSVデスクリプタヒープにシャドウマップをバインド
	int numDL = 0;
	for (const auto& it : dlsmInfo.dsvIndex) {
		for (int i = 0; i < kCascadeCount; i++) {
			//DSVハンドルゲット
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
			dsvHandle = ShadowMapRender::GetInstance()->GetDSVCPUDescriptorHandle(it[i]);
			//dsvDescのArrayの場所を選択
			dsvDesc.Texture2DArray.FirstArraySlice = numDL;
			//DSVとハンドルで指定した位置にDSVとリソースをバインドして格納
			DirectXCommon::GetInstance()->GetDevice()->CreateDepthStencilView(
				dlsmInfo.resource[i].Get(),
				&dsvDesc,
				dsvHandle
			);
		}
		numDL++;
	}

}

void ShadowMapManager::SettingSRV()
{
	//カスケードごとにSRVインデックスを割り当てる
	for (int j = 0; j < kCascadeCount; j++) {
		uint32_t srvIndex;
		srvIndex = SrvManager::GetInstance()->Allocate();
		dlsmInfo.srvIndex.push_back(srvIndex);
	}

	//SRVを作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;                   // フォーマット（シャドウマップは深度値）
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY; // 配列ビュー
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2DArray.ArraySize = kMaxNumDirectionalLight;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;                     // ミップレベル
	srvDesc.Texture2DArray.MostDetailedMip = 0;

	//カスケードごとにSRVデスクリプタヒープにシャドウマップをバインド
	for (int i = 0; i < kCascadeCount; i++) {
		//SRVのハンドルゲット
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
		srvHandle = SrvManager::GetInstance()->GetCPUDescriptorHandle(dlsmInfo.srvIndex[i]);
		//デスクリプタヒープにセット
		DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(
			dlsmInfo.resource[i].Get(),
			&srvDesc,
			srvHandle
		);
	}

}

void ShadowMapManager::InitLVPM()
{
	//リソースとデータのサイズを決める
	int size = (kMaxNumDirectionalLight * kCascadeCount) + (kMaxNumPointLight)+(kMaxNumSpotLight);
	//resize
	lightViewProjectionResources_.resize(size);
	lightViewProjectionDatas_.resize(size);
	lVPM_ = MyMath::MakeIdentity4x4();
	for (int i = 0; i < size; i++) {
		//リソースの作成
		lightViewProjectionResources_[i] = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(LightViewProjectionForVS));
		//リソースをマッピング
		lightViewProjectionResources_[i]->Map(0, nullptr, reinterpret_cast<void**>(&lightViewProjectionDatas_[i]));
		//データの初期化
		lightViewProjectionDatas_[i]->viewProjectionMatrix = lVPM_;
	}

}

void ShadowMapManager::GenerateGraphicsPipeline()
{
	HRESULT hr;

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootParameter作成。今回はカメラ行列、ライトの行列などが必要
	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	// オブジェクトのワールド行列
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	// ライト視点のビュープロジェクション
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 1;

	// Signatureに反映
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3D10Blob> signatireBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatireBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatireBlob->GetBufferPointer(),
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// InputLayoutの設定（シャドウマップの場合、通常のオブジェクト描画と同じ）
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendStateの設定（シャドウマップには不要な場合が多い）
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	// RasterizerStateの設定（シャドウマップのため裏面を描画しない）
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/ShadowMap.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = false;                        // ステンシルテストを無効化

	// グラフィックスパイプラインの設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { nullptr, 0 }; // ピクセルシェーダーは使用しない
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT; // 深度バッファの形式

	// 実際に生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
}
