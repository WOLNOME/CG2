#include "PostEffectManager.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "GPUDescriptorManager.h"
#include "RTVManager.h"
#include "DSVManager.h"
#include "MainRender.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "Logger.h"
#include <random>

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
	//固有リソースの初期化
	InitUniqueResources();
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
		kRenderTragetClearValue.x,
		kRenderTragetClearValue.y,
		kRenderTragetClearValue.z,
		kRenderTragetClearValue.w
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

	//ルートシグネチャ、パイプラインステートの設定
	commandList->SetGraphicsRootSignature(rootSignature[(int)currentPostEffectKind].Get());
	commandList->SetPipelineState(graphicsPipelineState[(int)currentPostEffectKind].Get());

	//個別描画情報記述
	switch (currentPostEffectKind) {
	case PostEffectKind::Dissolve:
		//マスクテクスチャ
		commandList->SetGraphicsRootDescriptorTable(1, GPUDescriptorManager::GetInstance()->GetGPUDescriptorHandle(TextureManager::GetInstance()->GetSrvIndex(postEffectResource.dissolveResource.textureHandle)));
		//ディゾルブデータ
		commandList->SetGraphicsRootConstantBufferView(2, postEffectResource.dissolveResource.resource->GetGPUVirtualAddress());
		break;
	case PostEffectKind::Random: {
		//ランダムエンジンを使ってシードを設定
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		postEffectResource.randomResource.data->seed = dist(gen);
		//ランダムデータ
		commandList->SetGraphicsRootConstantBufferView(1, postEffectResource.randomResource.resource->GetGPUVirtualAddress());
		break;
	}
	case PostEffectKind::HSVFilter:
		//HSVフィルターデータ
		commandList->SetGraphicsRootConstantBufferView(1, postEffectResource.hsvResource.resource->GetGPUVirtualAddress());
		break;
	default:
		break;
	}

	//共通描画情報記述
	commandList->SetGraphicsRootDescriptorTable(0, GPUDescriptorManager::GetInstance()->GetGPUDescriptorHandle(srvIndex));
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//ドローコール
	commandList->DrawInstanced(3, 1, 0, 0);

	//バリアの設定
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTextureResource.Get();		//レンダーテクスチャに対して行う
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;	//遷移前の状態
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;			//遷移後の状態
	commandList->ResourceBarrier(1, &barrier);
}

void PostEffectManager::DebugWithImGui() {
	ImGui::Begin("ポストエフェクト");
	//ポストエフェクトの種類を選択する
	const char* items[] = { "None","Grayscale","Vignette","BoxFilter","GaussianFilter","LuminanceBaseOutline","RadialBlur","Dissolve","Random","HSVFilter" };
	static int currentItem = 0;
	if (ImGui::Combo("一覧", &currentItem, items, IM_ARRAYSIZE(items))) {
		currentPostEffectKind = static_cast<PostEffectKind>(currentItem);
	}
	//各処理ごとの値編集
	switch (currentPostEffectKind) {
	case PostEffectKind::Dissolve: {
		//閾値の変更処理
		ImGui::DragFloat("しきい値", &postEffectResource.dissolveResource.data->threshold, 0.01f, 0.0f, 1.0f);
		//テクスチャの変更処理
		const char* textures[] = { "noise0","noise1" };
		static int currentTexture = 0;
		if (ImGui::Combo("テクスチャ", &currentTexture, textures, IM_ARRAYSIZE(textures))) {
			switch (currentTexture) {
			case 0:
				postEffectResource.dissolveResource.textureHandle = TextureManager::GetInstance()->LoadTexture("noise0.png");
				break;
			case 1:
				postEffectResource.dissolveResource.textureHandle = TextureManager::GetInstance()->LoadTexture("noise1.png");
				break;
			default:
				break;
			}
		}
		break;
	}
	case PostEffectKind::HSVFilter:
		ImGui::DragFloat("Hue", &postEffectResource.hsvResource.data->hsvColor.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Saturation", &postEffectResource.hsvResource.data->hsvColor.y, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("Value", &postEffectResource.hsvResource.data->hsvColor.z, 0.01f, -1.0f, 1.0f);

		break;
	default:
		break;
	}

	ImGui::End();
}

void PostEffectManager::InitOffScreenRenderingOption() {
	//RTVデスクリプタハンドルの取得
	rtvIndex = RTVManager::GetInstance()->Allocate();
	//RTVの作成
	renderTextureResource = DirectXCommon::GetInstance()->CreateRenderTextureResource(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTragetClearValue);
	RTVManager::GetInstance()->CreateRTVDescriptor(rtvIndex, renderTextureResource.Get());
	//SRVデスクリプタハンドルの取得
	srvIndex = GPUDescriptorManager::GetInstance()->Allocate();
	//SRVの作成
	GPUDescriptorManager::GetInstance()->CreateSRVforRenderTexture(srvIndex, renderTextureResource.Get());
}

void PostEffectManager::GenerateRenderTextureGraphicsPipeline() {
	HRESULT hr;
	//全てのポストエフェクト分のGraphicsPipelineを生成する
	for (int i = 0; i < (int)PostEffectKind::kMaxNumPostEffectKind; i++) {

		//RootSignature作成
		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
		descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


		//RootParameter格納用変数
		std::vector<D3D12_ROOT_PARAMETER> rootParameters;
		switch (i) {
		case (int)PostEffectKind::None:
		case (int)PostEffectKind::Grayscale:
		case (int)PostEffectKind::Vignette:
		case (int)PostEffectKind::BoxFilter:
		case (int)PostEffectKind::GaussianFilter:
		case (int)PostEffectKind::LuminanceBaseOutline:
		case (int)PostEffectKind::RadialBlur: {
			//RootParameter作成
			//レンダーテクスチャの設定
			{
				D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
				descriptorRange[0].BaseShaderRegister = 0;
				descriptorRange[0].NumDescriptors = 1;
				descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER rootParameter = {};
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Tableを使う
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
				rootParameter.DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
				rootParameter.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
				rootParameters.push_back(rootParameter);
			}
			break;
		}
		case (int)PostEffectKind::Dissolve: {
			//DescriptorRangeの配列を作成
			D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
			//レンダーテクスチャの設定
			descriptorRange[0].BaseShaderRegister = 0;
			descriptorRange[0].NumDescriptors = 1;
			descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			//マスクテクスチャの設定
			descriptorRange[1].BaseShaderRegister = 1;
			descriptorRange[1].NumDescriptors = 1;
			descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			//RootParameter作成
			//レンダーテクスチャの設定
			{
				D3D12_ROOT_PARAMETER rootParameter = {};
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Tableを使う
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
				rootParameter.DescriptorTable.pDescriptorRanges = &descriptorRange[0];//Tableの中身の配列を指定
				rootParameter.DescriptorTable.NumDescriptorRanges = 1;
				rootParameters.push_back(rootParameter);
			}
			//マスクテクスチャの設定
			{
				D3D12_ROOT_PARAMETER rootParameter = {};
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Tableを使う
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
				rootParameter.DescriptorTable.pDescriptorRanges = &descriptorRange[1];//Tableの中身の配列を指定
				rootParameter.DescriptorTable.NumDescriptorRanges = 1;
				rootParameters.push_back(rootParameter);
			}
			//ディゾルブデータの設定
			{
				D3D12_ROOT_PARAMETER rootParameter = {};
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
				rootParameter.Descriptor.ShaderRegister = 0;
				rootParameters.push_back(rootParameter);
			}
			break;
		}
		case (int)PostEffectKind::Random: {
			//RootParameter作成
			//レンダーテクスチャの設定
			{
				D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
				descriptorRange[0].BaseShaderRegister = 0;
				descriptorRange[0].NumDescriptors = 1;
				descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER rootParameter = {};
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Tableを使う
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
				rootParameter.DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
				rootParameter.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
				rootParameters.push_back(rootParameter);
			}
			//ランダムデータの設定
			{
				D3D12_ROOT_PARAMETER rootParameter = {};
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
				rootParameter.Descriptor.ShaderRegister = 0;
				rootParameters.push_back(rootParameter);
			}
			break;
		}
		case (int)PostEffectKind::HSVFilter: {
			//RootParameter作成
			//レンダーテクスチャの設定
			{
				D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
				descriptorRange[0].BaseShaderRegister = 0;
				descriptorRange[0].NumDescriptors = 1;
				descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER rootParameter = {};
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Tableを使う
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
				rootParameter.DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
				rootParameter.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
				rootParameters.push_back(rootParameter);
			}
			//HSVフィルターデータの設定
			{
				D3D12_ROOT_PARAMETER rootParameter = {};
				rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
				rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
				rootParameter.Descriptor.ShaderRegister = 0;
				rootParameters.push_back(rootParameter);
			}
			break;
		}


		default:
			break;
		}

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
		descriptionRootSignature.pParameters = rootParameters.data();//ルートパラメータ配列へのポインタ
		descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters.size());//配列の長さ

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
		case (int)PostEffectKind::Dissolve:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/Dissolve.PS.hlsl",
				L"ps_6_0");
			break;
		case (int)PostEffectKind::Random:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/Random.PS.hlsl",
				L"ps_6_0");
			break;
		case (int)PostEffectKind::HSVFilter:
			pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/postEffects/HSVFilter.PS.hlsl",
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

void PostEffectManager::InitUniqueResources() {
	//ディゾルブ
	postEffectResource.dissolveResource.resource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(DissolveResource));
	postEffectResource.dissolveResource.resource->Map(0, nullptr, reinterpret_cast<void**>(&postEffectResource.dissolveResource.data));
	postEffectResource.dissolveResource.data->threshold = 0.0f;
	postEffectResource.dissolveResource.textureHandle = TextureManager::GetInstance()->LoadTexture("noise0.png");
	//ランダム
	postEffectResource.randomResource.resource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(RandomResource));
	postEffectResource.randomResource.resource->Map(0, nullptr, reinterpret_cast<void**>(&postEffectResource.randomResource.data));
	postEffectResource.randomResource.data->seed = 0.0f;
	//HSVフィルター
	postEffectResource.hsvResource.resource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(HSVFilterResource));
	postEffectResource.hsvResource.resource->Map(0, nullptr, reinterpret_cast<void**>(&postEffectResource.hsvResource.data));
	postEffectResource.hsvResource.data->hsvColor = { 0.0f,0.0f,0.0f };
}
