#include "SkyboxCommon.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "Logger.h"
#include <vector>
#include <cassert>

SkyboxCommon* SkyboxCommon::instance = nullptr;

SkyboxCommon* SkyboxCommon::GetInstance() {
	if (instance == nullptr) {
		instance = new SkyboxCommon;
	}
	return instance;
}

void SkyboxCommon::Initialize() {
	// グラフィックスパイプラインの生成
	GenerateGraphicsPipeline();
}

void SkyboxCommon::Finalize() {
	delete instance;
	instance = nullptr;
}

void SkyboxCommon::SettingCommonDrawing() {
	// ルートシグネチャをセットするコマンド
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// グラフィックスパイプラインステートをセットするコマンド
	MainRender::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	// プリミティブトポロジーをセットするコマンド
	MainRender::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void SkyboxCommon::GenerateGraphicsPipeline() {
	// 必要な変数
	HRESULT hr;
	
	// RootSignature作成（使用するレジスタ : t0）
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// レジスタカウント
	int registerCount = 0;
	// 使用するデスクリプタの数
	int numDescriptors = 0;
	// テクスチャ用DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	// オブジェクトのテクスチャ用
	numDescriptors = 1;
	descriptorRange[0].BaseShaderRegister = registerCount;
	descriptorRange[0].NumDescriptors = numDescriptors;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCount += numDescriptors;

	//ルートパラメータ
	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	//ワールドトランスフォーム関連の設定
	D3D12_ROOT_PARAMETER worldTransformParam{};
	worldTransformParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	worldTransformParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	worldTransformParam.Descriptor.ShaderRegister = 0;
	rootParameters.push_back(worldTransformParam);
	//ビュープロジェクション関連の設定
	D3D12_ROOT_PARAMETER viewProjParam{};
	viewProjParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	viewProjParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	viewProjParam.Descriptor.ShaderRegister = 1;
	rootParameters.push_back(viewProjParam);
	//テクスチャの設定
	D3D12_ROOT_PARAMETER textureParam{};
	textureParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	textureParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	textureParam.DescriptorTable.pDescriptorRanges = descriptorRange;
	textureParam.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters.push_back(textureParam);
	
	// ルートシグネチャの記述
	descriptionRootSignature.pParameters = rootParameters.data(); // std::vectorのデータポインタを使用
	descriptionRootSignature.NumParameters = (UINT)rootParameters.size();

	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;

	// Signatureに反映
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3D10Blob> signatireBlob=nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatireBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとに生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatireBlob->GetBufferPointer(), signatireBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//インプットレイアウトの設定
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[1] = {};
	inputElementDesc[0].SemanticName = "POSITION";
	inputElementDesc[0].SemanticIndex = 0;
	inputElementDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	//インプットレイアウトディスクに登録
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = _countof(inputElementDesc);

	//BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	//全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;
	vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Skybox.VS.hlsl",L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;
	pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Skybox.PS.hlsl", L"ps_6_0");

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	

	//グラフィックスパイプラインステートの設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	//ラスタライザーステートの設定
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	//RTVの設定
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジのタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//DepthStencilStateの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	//グラフィックスパイプラインステートの設定
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//実際に生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));


}
