#include "Object3dCommon.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "SceneLight.h"
#include "Logger.h"

Object3dCommon* Object3dCommon::instance = nullptr;

Object3dCommon* Object3dCommon::GetInstance() {
	if (instance == nullptr) {
		instance = new Object3dCommon;
	}
	return instance;
}

void Object3dCommon::Initialize() {
	//グラフィックスパイプラインの生成
	GenerateGraphicsPipeline();
}

void Object3dCommon::Finalize() {
	delete instance;
	instance = nullptr;
}

void Object3dCommon::SettingCommonDrawing(NameGPS index) {
	//ルートシグネチャをセットするコマンド
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootSignature(rootSignature[(int)index].Get());
	//グラフィックスパイプラインステートをセットするコマンド
	MainRender::GetInstance()->GetCommandList()->SetPipelineState(graphicsPipelineState[(int)index].Get());
	//プリミティブトポロジーをセットするコマンド
	MainRender::GetInstance()->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dCommon::GenerateGraphicsPipeline() {
	//通常PSOの設定
	NormalPSOOption();
	//アニメーションPSOの設定
	AnimationPSOOption();
	//スカイボックス用PSOの設定
	SkyBoxPSOOption();

}

void Object3dCommon::NormalPSOOption() {
	HRESULT hr;
	//RootSignature作成（使用するレジスタ : t0）
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//レジスタカウント
	int registerCount = 0;
	//使用するデスクリプタの数
	int numDescriptors = 0;
	//テクスチャ用DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	D3D12_DESCRIPTOR_RANGE eltDescriptorRange[1] = {};
	//オブジェクトのテクスチャ用
	numDescriptors = 1;
	descriptorRange[0].BaseShaderRegister = registerCount;
	descriptorRange[0].NumDescriptors = numDescriptors;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCount += numDescriptors;
	//環境光テクスチャ用
	numDescriptors = 1;
	eltDescriptorRange[0].BaseShaderRegister = registerCount;
	eltDescriptorRange[0].NumDescriptors = numDescriptors;
	eltDescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	eltDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCount += numDescriptors;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	// マテリアルの設定(0)
	D3D12_ROOT_PARAMETER materialParam = {};
	materialParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	materialParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	materialParam.Descriptor.ShaderRegister = 0;
	rootParameters.push_back(materialParam);

	// ワールドトランスフォーム関連の設定(1)
	D3D12_ROOT_PARAMETER worldTransformParam = {};
	worldTransformParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	worldTransformParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	worldTransformParam.Descriptor.ShaderRegister = 0;
	rootParameters.push_back(worldTransformParam);

	// ビュープロジェクション関連の設定(2)
	D3D12_ROOT_PARAMETER viewProjParam = {};
	viewProjParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	viewProjParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	viewProjParam.Descriptor.ShaderRegister = 1;
	rootParameters.push_back(viewProjParam);

	// テクスチャの設定(3)
	D3D12_ROOT_PARAMETER textureParam = {};
	textureParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	textureParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	textureParam.DescriptorTable.pDescriptorRanges = descriptorRange;
	textureParam.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters.push_back(textureParam);

	// カメラ座標用定数バッファの設定(4)
	D3D12_ROOT_PARAMETER cameraParam = {};
	cameraParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	cameraParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	cameraParam.Descriptor.ShaderRegister = 1;
	rootParameters.push_back(cameraParam);

	// シーンライト用の設定(5)
	D3D12_ROOT_PARAMETER lightParam = {};
	lightParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	lightParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	lightParam.Descriptor.ShaderRegister = 2;
	rootParameters.push_back(lightParam);

	// 光源有無用の設定(6)
	D3D12_ROOT_PARAMETER lightExistParam = {};
	lightExistParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	lightExistParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	lightExistParam.Descriptor.ShaderRegister = 3;
	rootParameters.push_back(lightExistParam);

	// 環境マップテクスチャ用の設定(7)
	D3D12_ROOT_PARAMETER elTextureParam = {};
	elTextureParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	elTextureParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	elTextureParam.DescriptorTable.pDescriptorRanges = eltDescriptorRange;
	elTextureParam.DescriptorTable.NumDescriptorRanges = _countof(eltDescriptorRange);
	rootParameters.push_back(elTextureParam);

	// ルートシグネチャの記述
	descriptionRootSignature.pParameters = rootParameters.data(); // std::vectorのデータポインタを使用
	descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters.size()); // 要素数を取得

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
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature[(int)NameGPS::None]));
	assert(SUCCEEDED(hr));

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc;

	//宣言
	D3D12_INPUT_ELEMENT_DESC ied0 = {};
	D3D12_INPUT_ELEMENT_DESC ied1 = {};
	D3D12_INPUT_ELEMENT_DESC ied2 = {};
	//定義
	ied0.SemanticName = "POSITION";
	ied0.SemanticIndex = 0;
	ied0.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	ied0.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	ied1.SemanticName = "TEXCOORD";
	ied1.SemanticIndex = 0;
	ied1.Format = DXGI_FORMAT_R32G32_FLOAT;
	ied1.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	ied2.SemanticName = "NORMAL";
	ied2.SemanticIndex = 0;
	ied2.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	ied2.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//登録
	inputElementDesc.push_back(ied0);
	inputElementDesc.push_back(ied1);
	inputElementDesc.push_back(ied2);

	//インプットレイアウトディスクに登録
	inputLayoutDesc.pInputElementDescs = inputElementDesc.data();
	inputLayoutDesc.NumElements = static_cast<UINT>(inputElementDesc.size());

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
	vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Object3d.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
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

	//PSO情報を書き込む
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature[(int)NameGPS::None].Get();
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
		IID_PPV_ARGS(&graphicsPipelineState[(int)NameGPS::None]));
	assert(SUCCEEDED(hr));
}

void Object3dCommon::AnimationPSOOption() {
	HRESULT hr;
	//RootSignature作成（使用するレジスタ : t0）
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//レジスタカウント
	int registerCount = 0;
	//使用するデスクリプタの数
	int numDescriptors = 0;
	//テクスチャ用DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	D3D12_DESCRIPTOR_RANGE eltDescriptorRange[1] = {};
	//オブジェクトのテクスチャ用
	numDescriptors = 1;
	descriptorRange[0].BaseShaderRegister = registerCount;
	descriptorRange[0].NumDescriptors = numDescriptors;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCount += numDescriptors;
	//環境光テクスチャ用
	numDescriptors = 1;
	eltDescriptorRange[0].BaseShaderRegister = registerCount;
	eltDescriptorRange[0].NumDescriptors = numDescriptors;
	eltDescriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	eltDescriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCount += numDescriptors;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;

	// マテリアルの設定(0)
	D3D12_ROOT_PARAMETER materialParam = {};
	materialParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	materialParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	materialParam.Descriptor.ShaderRegister = 0;
	rootParameters.push_back(materialParam);

	// ワールドトランスフォーム関連の設定(1)
	D3D12_ROOT_PARAMETER worldTransformParam = {};
	worldTransformParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	worldTransformParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	worldTransformParam.Descriptor.ShaderRegister = 0;
	rootParameters.push_back(worldTransformParam);

	// ビュープロジェクション関連の設定(2)
	D3D12_ROOT_PARAMETER viewProjParam = {};
	viewProjParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	viewProjParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	viewProjParam.Descriptor.ShaderRegister = 1;
	rootParameters.push_back(viewProjParam);

	// テクスチャの設定(3)
	D3D12_ROOT_PARAMETER textureParam = {};
	textureParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	textureParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	textureParam.DescriptorTable.pDescriptorRanges = descriptorRange;
	textureParam.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters.push_back(textureParam);

	// カメラ座標用定数バッファの設定(4)
	D3D12_ROOT_PARAMETER cameraParam = {};
	cameraParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	cameraParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	cameraParam.Descriptor.ShaderRegister = 1;
	rootParameters.push_back(cameraParam);

	// シーンライト用の設定(5)
	D3D12_ROOT_PARAMETER lightParam = {};
	lightParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	lightParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	lightParam.Descriptor.ShaderRegister = 2;
	rootParameters.push_back(lightParam);

	// 光源有無用の設定(6)
	D3D12_ROOT_PARAMETER lightExistParam = {};
	lightExistParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	lightExistParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	lightExistParam.Descriptor.ShaderRegister = 3;
	rootParameters.push_back(lightExistParam);

	// 環境光テクスチャ用の設定(7)
	D3D12_ROOT_PARAMETER elTextureParam = {};
	elTextureParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	elTextureParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	elTextureParam.DescriptorTable.pDescriptorRanges = eltDescriptorRange;
	elTextureParam.DescriptorTable.NumDescriptorRanges = _countof(eltDescriptorRange);
	rootParameters.push_back(elTextureParam);

	//MatrixPalette用の設定(8)
	D3D12_ROOT_PARAMETER matrixPaletteParam = {};
	matrixPaletteParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	matrixPaletteParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	matrixPaletteParam.DescriptorTable.pDescriptorRanges = descriptorRange;
	matrixPaletteParam.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters.push_back(matrixPaletteParam);

	// ルートシグネチャの記述
	descriptionRootSignature.pParameters = rootParameters.data(); // std::vectorのデータポインタを使用
	descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters.size()); // 要素数を取得

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
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature[(int)NameGPS::Animation]));
	assert(SUCCEEDED(hr));

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc;

	//宣言
	D3D12_INPUT_ELEMENT_DESC ied0 = {};
	D3D12_INPUT_ELEMENT_DESC ied1 = {};
	D3D12_INPUT_ELEMENT_DESC ied2 = {};
	D3D12_INPUT_ELEMENT_DESC ied3 = {};
	D3D12_INPUT_ELEMENT_DESC ied4 = {};
	//定義
	ied0.SemanticName = "POSITION";
	ied0.SemanticIndex = 0;
	ied0.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	ied0.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	ied1.SemanticName = "TEXCOORD";
	ied1.SemanticIndex = 0;
	ied1.Format = DXGI_FORMAT_R32G32_FLOAT;
	ied1.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	ied2.SemanticName = "NORMAL";
	ied2.SemanticIndex = 0;
	ied2.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	ied2.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	ied3.SemanticName = "WEIGHT";
	ied3.SemanticIndex = 0;
	ied3.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	ied3.InputSlot = 1;//1番目のslotのVBVの事だと伝える
	ied3.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	ied4.SemanticName = "INDEX";
	ied4.SemanticIndex = 0;
	ied4.Format = DXGI_FORMAT_R32G32B32A32_SINT;
	ied4.InputSlot = 1;//1番目のslotのVBVの事だと伝える
	ied4.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//登録
	inputElementDesc.push_back(ied0);
	inputElementDesc.push_back(ied1);
	inputElementDesc.push_back(ied2);
	inputElementDesc.push_back(ied3);
	inputElementDesc.push_back(ied4);

	//インプットレイアウトディスクに登録
	inputLayoutDesc.pInputElementDescs = inputElementDesc.data();
	inputLayoutDesc.NumElements = static_cast<UINT>(inputElementDesc.size());

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

	vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/SkinningObject3d.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

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
	graphicsPipelineStateDesc.pRootSignature = rootSignature[(int)NameGPS::Animation].Get();
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
		IID_PPV_ARGS(&graphicsPipelineState[(int)NameGPS::Animation]));
	assert(SUCCEEDED(hr));
}

void Object3dCommon::SkyBoxPSOOption() {
	HRESULT hr;
	//RootSignature作成（使用するレジスタ : t0）
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//レジスタカウント
	int registerCount = 0;
	//使用するデスクリプタの数
	int numDescriptors = 0;
	//テクスチャ用DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	//オブジェクトのテクスチャ用
	numDescriptors = 6;
	descriptorRange[0].BaseShaderRegister = registerCount;
	descriptorRange[0].NumDescriptors = numDescriptors;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCount += numDescriptors;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	// マテリアルの設定
	D3D12_ROOT_PARAMETER materialParam = {};
	materialParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	materialParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	materialParam.Descriptor.ShaderRegister = 0;
	rootParameters.push_back(materialParam);

	// ワールドトランスフォーム関連の設定
	D3D12_ROOT_PARAMETER worldTransformParam = {};
	worldTransformParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	worldTransformParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	worldTransformParam.Descriptor.ShaderRegister = 0;
	rootParameters.push_back(worldTransformParam);

	// ビュープロジェクション関連の設定
	D3D12_ROOT_PARAMETER viewProjParam = {};
	viewProjParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	viewProjParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	viewProjParam.Descriptor.ShaderRegister = 1;
	rootParameters.push_back(viewProjParam);

	// テクスチャの設定
	D3D12_ROOT_PARAMETER textureParam = {};
	textureParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	textureParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	textureParam.DescriptorTable.pDescriptorRanges = descriptorRange;
	textureParam.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	rootParameters.push_back(textureParam);

	// カメラ座標用定数バッファの設定
	D3D12_ROOT_PARAMETER cameraParam = {};
	cameraParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	cameraParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	cameraParam.Descriptor.ShaderRegister = 1;
	rootParameters.push_back(cameraParam);

	// シーンライト用の設定
	D3D12_ROOT_PARAMETER lightParam = {};
	lightParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	lightParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	lightParam.Descriptor.ShaderRegister = 2;
	rootParameters.push_back(lightParam);

	// 光源有無用の設定
	D3D12_ROOT_PARAMETER lightExistParam = {};
	lightExistParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	lightExistParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	lightExistParam.Descriptor.ShaderRegister = 3;
	rootParameters.push_back(lightExistParam);

	// ルートシグネチャの記述
	descriptionRootSignature.pParameters = rootParameters.data(); // std::vectorのデータポインタを使用
	descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters.size()); // 要素数を取得

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
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature[(int)NameGPS::SkyBox]));
	assert(SUCCEEDED(hr));

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc;

	//宣言
	D3D12_INPUT_ELEMENT_DESC ied0 = {};
	D3D12_INPUT_ELEMENT_DESC ied1 = {};
	D3D12_INPUT_ELEMENT_DESC ied2 = {};
	//定義
	ied0.SemanticName = "POSITION";
	ied0.SemanticIndex = 0;
	ied0.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	ied0.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	ied1.SemanticName = "TEXCOORD";
	ied1.SemanticIndex = 0;
	ied1.Format = DXGI_FORMAT_R32G32_FLOAT;
	ied1.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	ied2.SemanticName = "NORMAL";
	ied2.SemanticIndex = 0;
	ied2.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	ied2.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	//登録
	inputElementDesc.push_back(ied0);
	inputElementDesc.push_back(ied1);
	inputElementDesc.push_back(ied2);

	//インプットレイアウトディスクに登録
	inputLayoutDesc.pInputElementDescs = inputElementDesc.data();
	inputLayoutDesc.NumElements = static_cast<UINT>(inputElementDesc.size());

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
	vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Skybox.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Skybox.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	//PSO情報を書き込む
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature[(int)NameGPS::SkyBox].Get();
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
		IID_PPV_ARGS(&graphicsPipelineState[(int)NameGPS::SkyBox]));
	assert(SUCCEEDED(hr));
}

