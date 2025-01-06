#include "Object3dCommon.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "SceneLight.h"
#include "Logger.h"

Object3dCommon* Object3dCommon::instance = nullptr;

Object3dCommon* Object3dCommon::GetInstance()
{
	if (instance == nullptr) {
		instance = new Object3dCommon;
	}
	return instance;
}

void Object3dCommon::Initialize()
{
	//グラフィックスパイプラインの生成
	GenerateGraphicsPipeline();
}

void Object3dCommon::Finalize()
{
	delete instance;
	instance = nullptr;
}

void Object3dCommon::SettingCommonDrawing(NameGPS index)
{
	//ルートシグネチャをセットするコマンド
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature[index].Get());
	//グラフィックスパイプラインステートをセットするコマンド
	MainRender::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState[index].Get());
	//プリミティブトポロジーをセットするコマンド
	MainRender::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dCommon::GenerateGraphicsPipeline()
{
	for (uint32_t index = 0; index < kNumGraphicsPipeline; ++index) {
		HRESULT hr;

		// RootSignature作成
		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature = {};
		descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		// DescriptorRange作成
		D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
		descriptorRange[0].BaseShaderRegister = 0;
		descriptorRange[0].NumDescriptors = 1;
		descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// RootParameter作成
		D3D12_ROOT_PARAMETER rootParameters[8] = {}; // 最大で8個のパラメータを使用
		uint32_t rootParameterCount = 0;

		if (index == NameGPS::None) {
			// マテリアルの設定
			rootParameters[rootParameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParameters[rootParameterCount].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			rootParameters[rootParameterCount].Descriptor.ShaderRegister = 0;
			rootParameterCount++;

			// ワールドトランスフォーム
			rootParameters[rootParameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParameters[rootParameterCount].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParameters[rootParameterCount].Descriptor.ShaderRegister = 0;
			rootParameterCount++;

			// ビュープロジェクション
			rootParameters[rootParameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParameters[rootParameterCount].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParameters[rootParameterCount].Descriptor.ShaderRegister = 1;
			rootParameterCount++;

			// テクスチャの設定
			rootParameters[rootParameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameters[rootParameterCount].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			rootParameters[rootParameterCount].DescriptorTable.pDescriptorRanges = descriptorRange;
			rootParameters[rootParameterCount].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
			rootParameterCount++;

			// その他の設定（カメラ座標、光源など）
			// 必要に応じて追加します...
		}
		else if (index == NameGPS::Animation) {
			// Animation用の追加パラメータを設定
			rootParameters[rootParameterCount].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameters[rootParameterCount].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
			rootParameters[rootParameterCount].DescriptorTable.pDescriptorRanges = descriptorRange;
			rootParameters[rootParameterCount].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
			rootParameterCount++;
		}

		// ルートシグネチャ設定
		descriptionRootSignature.pParameters = rootParameters;
		descriptionRootSignature.NumParameters = rootParameterCount;

		// Static Samplerの設定
		D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
		staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
		staticSamplers[0].ShaderRegister = 0;
		staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		descriptionRootSignature.pStaticSamplers = staticSamplers;
		descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

		// シリアライズしてバイナリにする
		Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
		hr = D3D12SerializeRootSignature(&descriptionRootSignature,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&signatureBlob, &errorBlob);
		if (FAILED(hr)) {
			if (errorBlob) {
				const char* errorMessage = reinterpret_cast<const char*>(errorBlob->GetBufferPointer());
				Logger::Log(errorMessage); // エラーメッセージをログに出力
			}
			assert(false);
		}

		// RootSignatureを生成
		hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(0,
			signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&rootSignature[index]));
		assert(SUCCEEDED(hr));

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		if (index == NameGPS::None) {
			//InputLayout
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
			inputElementDescs[0].SemanticName = "POSITION";
			inputElementDescs[0].SemanticIndex = 0;
			inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			inputElementDescs[1].SemanticName = "TEXCOORD";
			inputElementDescs[1].SemanticIndex = 0;
			inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
			inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			inputElementDescs[2].SemanticName = "NORMAL";
			inputElementDescs[2].SemanticIndex = 0;
			inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			//インプットレイアウトディスクに登録
			inputLayoutDesc.pInputElementDescs = inputElementDescs;
			inputLayoutDesc.NumElements = _countof(inputElementDescs);
		}
		else if (index == NameGPS::Animation) {
			//InputLayout
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[5] = {};
			inputElementDescs[0].SemanticName = "POSITION";
			inputElementDescs[0].SemanticIndex = 0;
			inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			inputElementDescs[1].SemanticName = "TEXCOORD";
			inputElementDescs[1].SemanticIndex = 0;
			inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
			inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			inputElementDescs[2].SemanticName = "NORMAL";
			inputElementDescs[2].SemanticIndex = 0;
			inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			inputElementDescs[3].SemanticName = "WEIGHT";
			inputElementDescs[3].SemanticIndex = 0;
			inputElementDescs[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			inputElementDescs[3].InputSlot = 1;//1番目のslotのVBVの事だと伝える
			inputElementDescs[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			inputElementDescs[4].SemanticName = "INDEX";
			inputElementDescs[4].SemanticIndex = 0;
			inputElementDescs[4].Format = DXGI_FORMAT_R32G32B32A32_SINT;
			inputElementDescs[4].InputSlot = 1;//1番目のslotのVBVの事だと伝える
			inputElementDescs[4].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
			//インプットレイアウトディスクに登録
			inputLayoutDesc.pInputElementDescs = inputElementDescs;
			inputLayoutDesc.NumElements = _countof(inputElementDescs);
		}
		
		//BlendSyayeの設定
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
		switch (index)
		{
		case Object3dCommon::None:
			vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Object3d.VS.hlsl",
				L"vs_6_0");
			assert(vertexShaderBlob != nullptr);
			break;
		case Object3dCommon::Animation:
			vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/SkinningObject3d.VS.hlsl",
				L"vs_6_0");
			assert(vertexShaderBlob != nullptr);
			break;
		default:
			break;
		}

		Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Object3d.PS.hlsl",
			L"ps_6_0");
		assert(pixelShaderBlob != nullptr);

		//DepthStencilStateの設定
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
		//Depthの機能を有効化する
		depthStencilDesc.DepthEnable = true;
		//書き込みします
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		//比較関数はLessEqual。つまり、近ければ描画される
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
		graphicsPipelineStateDesc.pRootSignature = rootSignature[index].Get();
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
			IID_PPV_ARGS(&graphicsPipelineState[index]));
		assert(SUCCEEDED(hr));
	}
}
