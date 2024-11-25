#include "ShadowMapGenerator.h"
#include "DirectXCommon.h"
#include "Logger.h"
#include <cassert>

ShadowMapGenerator* ShadowMapGenerator::instance = nullptr;

ShadowMapGenerator* ShadowMapGenerator::GetInstance()
{
	if (instance == nullptr) {
		instance = new ShadowMapGenerator;
	}
	return instance;
}

void ShadowMapGenerator::Initialize()
{
	//グラフィックスパイプラインの設定
	GenerateGraphicsPipeline();
}

void ShadowMapGenerator::Finalize()
{
	delete instance;
	instance = nullptr;
}

void ShadowMapGenerator::SettingCommonDrawing()
{
	//ルートシグネチャをセットするコマンド
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	//グラフィックスパイプラインステートをセットするコマンド
	DirectXCommon::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	//プリミティブトポロジーをセットするコマンド
	DirectXCommon::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ShadowMapGenerator::GenerateGraphicsPipeline()
{
	HRESULT hr;

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// RootParameter作成。今回はカメラ行列、ライトの行列などが必要
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
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
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
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
