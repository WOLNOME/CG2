#include "PostEffectManager.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "GPUDescriptorManager.h"
#include "RTVManager.h"
#include "DSVManager.h"
#include "MainRender.h"
#include "ImGuiManager.h"
#include "Logger.h"

PostEffectManager* PostEffectManager::instance = nullptr;

using namespace Microsoft::WRL;

PostEffectManager* PostEffectManager::GetInstance() {
	if (instance == nullptr) {
		instance = new PostEffectManager;
	}
	return instance;
}

void PostEffectManager::Initialize() {
	//オフスク用レンダーテクスチャの生成
	InitOffScreenRenderingOption();
	//オフスク用グラフィックスパイプラインの生成
	GenerateRenderTextureGraphicsPipeline();
}

void PostEffectManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void PostEffectManager::PreObjectDraw() {
	//メインレンダーのインスタンスを取得
	MainRender* mainRender = MainRender::GetInstance();
	//コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = mainRender->GetCommandList();
	//描画先のRTVとDSVを設定するを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RTVManager::GetInstance()->GetCPUDescriptorHandle(rtvIndex);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DSVManager::GetInstance()->GetCPUDescriptorHandle(mainRender->GetDSVIndex());
	commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
	//クリアバリューの色で画面全体をクリアする
	float clearColor[] = {
		DirectXCommon::GetInstance()->GetClearValue().Color[0],
		DirectXCommon::GetInstance()->GetClearValue().Color[1],
		DirectXCommon::GetInstance()->GetClearValue().Color[2],
		DirectXCommon::GetInstance()->GetClearValue().Color[3]
	};
	commandList->ClearRenderTargetView(RTVManager::GetInstance()->GetCPUDescriptorHandle(rtvIndex), clearColor, 0, nullptr);
	//指定した深度で画面全体をクリアする
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void PostEffectManager::CopySceneToRenderTexture() {
	//コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = MainRender::GetInstance()->GetCommandList();
	//バリアの設定
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTextureResource.Get();		//レンダーテクスチャに対して行う
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;			//遷移前の状態
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;		//遷移後の状態
	commandList->ResourceBarrier(1, &barrier);

	//描画情報コピー
	commandList->SetGraphicsRootSignature(rootSignature[(int)currentPostEffectKind].Get());
	commandList->SetPipelineState(graphicsPipelineState[(int)currentPostEffectKind].Get());
	commandList->SetGraphicsRootDescriptorTable(0, GPUDescriptorManager::GetInstance()->GetGPUDescriptorHandle(srvIndex));
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

void PostEffectManager::DebugWithImGui() {
	ImGui::Begin("ポストエフェクト");
	//ポストエフェクトの種類を選択する
	const char* items[] = { "None","Grayscale","Vignette","BoxFilter","GaussianFilter","LuminanceBaseOutline","RadialBlur" };
	static int currentItem = 0;
	if (ImGui::Combo("アイテム", &currentItem, items, IM_ARRAYSIZE(items))) {
		currentPostEffectKind = static_cast<PostEffectKind>(currentItem);
	}
	ImGui::End();
}

void PostEffectManager::InitOffScreenRenderingOption() {
	//RTVディスクリプタハンドルの取得
	rtvIndex = RTVManager::GetInstance()->Allocate();
	//RTVの作成
	const Vector4 kRenderTragetClearValue = Vector4(1, 1, 0, 0);
	renderTextureResource = DirectXCommon::GetInstance()->CreateRenderTextureResource(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTragetClearValue);
	RTVManager::GetInstance()->CreateRTVDescriptor(rtvIndex, renderTextureResource.Get());
	//SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC renderTextureSrvDesc{};
	renderTextureSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	renderTextureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	renderTextureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	renderTextureSrvDesc.Texture2D.MipLevels = 1;
	srvIndex = GPUDescriptorManager::GetInstance()->Allocate();
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(renderTextureResource.Get(), &renderTextureSrvDesc, GPUDescriptorManager::GetInstance()->GetCPUDescriptorHandle(srvIndex));
}

void PostEffectManager::GenerateRenderTextureGraphicsPipeline() {
	HRESULT hr;
	//全てのポストエフェクト分のGraphicsPipelineを生成する
	for (int i = 0; i < (int)PostEffectKind::kMaxNumPostEffectKind; i++) {

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
			signatireBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature[i]));
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
		Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/CopyImage.VS.hlsl",
			L"vs_6_0");
		assert(vertexShaderBlob != nullptr);

		//ポストエフェクトに応じてPSを変更する
		Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;
		switch (i) {
		case (int)PostEffectKind::None:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/CopyImage.PS.hlsl",
				L"ps_6_0");
			break;
		case (int)PostEffectKind::Grayscale:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/Grayscale.PS.hlsl",
				L"ps_6_0");
			break;
		case (int)PostEffectKind::Vignette:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/Vignette.PS.hlsl",
				L"ps_6_0");
			break;
		case (int)PostEffectKind::BoxFilter:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/BoxFilter.PS.hlsl",
				L"ps_6_0");
			break;
		case (int)PostEffectKind::GaussianFilter:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/GaussianFilter.PS.hlsl",
				L"ps_6_0");
			break;
		case (int)PostEffectKind::LuminanceBaseOutline:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/LuminanceBasedOutline.PS.hlsl",
				L"ps_6_0");
			break;
		case (int)PostEffectKind::RadialBlur:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/RadialBlur.PS.hlsl",
				L"ps_6_0");
			break;
		default:
			break;
		}
		assert(pixelShaderBlob != nullptr);
		
		//DepthStencilStateの設定
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		//Depthの機能を無効化する
		depthStencilDesc.DepthEnable = false;

		//グラフィックスパイプラインステートに設定を反映
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
		graphicsPipelineStateDesc.pRootSignature = rootSignature[i].Get();
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
			IID_PPV_ARGS(&graphicsPipelineState[i]));
		assert(SUCCEEDED(hr));
	}
}
