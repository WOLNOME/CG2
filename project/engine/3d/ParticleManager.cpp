#include "ParticleManager.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "GPUDescriptorManager.h"
#include "Logger.h"
#include "RandomStringUtil.h"
#include <numbers>
#include <random>
#undef min
#undef max
#include <algorithm>

ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance() {
	if (instance == nullptr) {
		instance = new ParticleManager;
	}
	return instance;
}

void ParticleManager::Initialize() {
	//グラフィックスパイプラインの設定
	GenerateGraphicsPipeline();
	//コンピュートパイプラインの設定
	GenerateComputePipeline();

}

void ParticleManager::Update() {
	MainRender* mainRender = MainRender::GetInstance();
	GPUDescriptorManager* gpuDescriptorManager = GPUDescriptorManager::GetInstance();
	auto InsertUAVBarriers = [&](std::vector<ID3D12Resource*>& resources) {
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		for (ID3D12Resource* resource : resources) {
			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.UAV.pResource = resource;
			barriers.push_back(barrier);
		}
		mainRender->GetCommandList()->ResourceBarrier(UINT(barriers.size()), barriers.data());
		};

	//GPUDescriptorHeapをコマンドリストにセット
	gpuDescriptorManager->SetDescriptorHeap(mainRender->GetCommandList());

	//各パーティクルの更新
	for (const auto& particle : particles) {
		//perFrameのタイムを更新
		particle.second->allResourceForCS_.mappedPerFrame[0].time += kDeltaTime;
		//エミッターとJSONデータの更新
		particle.second->TraceEmitterForCS();
		particle.second->TraceJsonDataForCS();
		//バリア遷移(状態保証処理)
		{
			std::vector<ID3D12Resource*> uavResources = {
				particle.second->allResourceForCS_.grainsResource.Get(),
				particle.second->allResourceForCS_.freeListIndexResource.Get(),
				particle.second->allResourceForCS_.freeListResource.Get()
			};

			InsertUAVBarriers(uavResources);
		}
		//粒の生成処理
		mainRender->GetCommandList()->SetComputeRootSignature(cRootSignature[1].Get());
		mainRender->GetCommandList()->SetPipelineState(computePipelineState[1].Get());
		mainRender->GetCommandList()->SetComputeRootDescriptorTable(0, gpuDescriptorManager->GetGPUDescriptorHandle(particle.second->allResourceForCS_.grainsUavIndex));
		mainRender->GetCommandList()->SetComputeRootDescriptorTable(1, gpuDescriptorManager->GetGPUDescriptorHandle(particle.second->allResourceForCS_.freeListIndexUavIndex));
		mainRender->GetCommandList()->SetComputeRootDescriptorTable(2, gpuDescriptorManager->GetGPUDescriptorHandle(particle.second->allResourceForCS_.freeListUavIndex));
		mainRender->GetCommandList()->SetComputeRootConstantBufferView(3, particle.second->allResourceForCS_.emitterResource->GetGPUVirtualAddress());
		mainRender->GetCommandList()->SetComputeRootConstantBufferView(4, particle.second->allResourceForCS_.jsonInfoResource->GetGPUVirtualAddress());
		mainRender->GetCommandList()->SetComputeRootConstantBufferView(5, particle.second->allResourceForCS_.perFrameResource->GetGPUVirtualAddress());

		mainRender->GetCommandList()->Dispatch(1, 1, 1);
		//OneShotスタイルの場合isPlayをfalseにする
		if (particle.second->emitter_.effectStyle == Particle::EffectStyle::OneShot) {
			particle.second->emitter_.isPlay = false;
		}
		//バリア遷移(状態保証処理)
		{
			std::vector<ID3D12Resource*> uavResources = {
				particle.second->allResourceForCS_.grainsResource.Get(),
				particle.second->allResourceForCS_.freeListIndexResource.Get(),
				particle.second->allResourceForCS_.freeListResource.Get()
			};

			InsertUAVBarriers(uavResources);
		}
		//粒の更新処理
		mainRender->GetCommandList()->SetComputeRootSignature(cRootSignature[2].Get());
		mainRender->GetCommandList()->SetPipelineState(computePipelineState[2].Get());
		mainRender->GetCommandList()->SetComputeRootDescriptorTable(0, gpuDescriptorManager->GetGPUDescriptorHandle(particle.second->allResourceForCS_.grainsUavIndex));
		mainRender->GetCommandList()->SetComputeRootDescriptorTable(1, gpuDescriptorManager->GetGPUDescriptorHandle(particle.second->allResourceForCS_.freeListIndexUavIndex));
		mainRender->GetCommandList()->SetComputeRootDescriptorTable(2, gpuDescriptorManager->GetGPUDescriptorHandle(particle.second->allResourceForCS_.freeListUavIndex));
		mainRender->GetCommandList()->SetComputeRootConstantBufferView(3, particle.second->allResourceForCS_.emitterResource->GetGPUVirtualAddress());
		mainRender->GetCommandList()->SetComputeRootConstantBufferView(4, particle.second->allResourceForCS_.jsonInfoResource->GetGPUVirtualAddress());
		mainRender->GetCommandList()->SetComputeRootConstantBufferView(5, particle.second->allResourceForCS_.perFrameResource->GetGPUVirtualAddress());

		mainRender->GetCommandList()->Dispatch(UINT(particle.second->param_["MaxGrains"] + 1023) / 1024, 1, 1);
		//粒配列情報をSRV用にリソース遷移
		mainRender->TransitionResource(particle.second->allResourceForCS_.grainsResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);

		//形状更新
		particle.second->shape_->Update();
	}
}

void ParticleManager::Draw() {
	//パーティクルが一つもセットされてなかったら抜ける
	if (particles.empty()) return;

	auto mainRender = MainRender::GetInstance();
	//カメラの有無チェック
	if (!camera_) {
		assert(0 && "カメラがセットされていません。");
	}
	//ルートシグネチャをセットするコマンド
	mainRender->GetCommandList()->SetGraphicsRootSignature(gRootSignature.Get());
	//プリミティブトポロジーをセットするコマンド
	mainRender->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//カメラ情報をVSに送信(一括)
	mainRender->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera_->GetViewProjectionConstBuffer()->GetGPUVirtualAddress());
	//パーティクル個別の設定
	for (const auto& particle : particles) {
		//各パーティクルのブレンドモード情報からパイプラインステートを選択
		mainRender->GetCommandList()->SetPipelineState(graphicsPipelineState[particle.second->GetParam()["BlendMode"]].Get());
		//各パーティクルの粒配列情報をVSに送信
		mainRender->GetCommandList()->SetGraphicsRootDescriptorTable(1, GPUDescriptorManager::GetInstance()->GetGPUDescriptorHandle(particle.second->allResourceForCS_.grainsSrvIndex));
		//エミッター情報をVSに送信
		mainRender->GetCommandList()->SetGraphicsRootConstantBufferView(4, particle.second->allResourceForCS_.emitterResource->GetGPUVirtualAddress());
		//JSON情報をVSに送信
		mainRender->GetCommandList()->SetGraphicsRootConstantBufferView(5, particle.second->allResourceForCS_.jsonInfoResource->GetGPUVirtualAddress());
		//各パーティクル形状の描画
		particle.second->shape_->Draw(0, 3, (uint32_t)particle.second->param_["MaxGrains"], particle.second->textureHandle_);

		//粒配列情報をUAV用にリソース遷移
		mainRender->TransitionResource(particle.second->allResourceForCS_.grainsResource.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
}

void ParticleManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void ParticleManager::RegisterParticle(const std::string& name, Particle* particle) {
	//重複チェック
	if (particles.find(name) != particles.end()) {
		return;
	}
	//登録
	particles[name] = particle;

	MainRender* mainRender = MainRender::GetInstance();
	GPUDescriptorManager* gpuDescriptorManager = GPUDescriptorManager::GetInstance();
	//GPUDescriptorHeapをコマンドリストにセット
	gpuDescriptorManager->SetDescriptorHeap(mainRender->GetCommandList());
	//CSでUAVリソースの初期化処理
	mainRender->GetCommandList()->SetComputeRootSignature(cRootSignature[0].Get());
	mainRender->GetCommandList()->SetPipelineState(computePipelineState[0].Get());
	mainRender->GetCommandList()->SetComputeRootDescriptorTable(0, gpuDescriptorManager->GetGPUDescriptorHandle(particles[name]->allResourceForCS_.grainsUavIndex));
	mainRender->GetCommandList()->SetComputeRootDescriptorTable(1, gpuDescriptorManager->GetGPUDescriptorHandle(particles[name]->allResourceForCS_.freeListIndexUavIndex));
	mainRender->GetCommandList()->SetComputeRootDescriptorTable(2, gpuDescriptorManager->GetGPUDescriptorHandle(particles[name]->allResourceForCS_.freeListUavIndex));
	mainRender->GetCommandList()->SetComputeRootConstantBufferView(3, particles[name]->allResourceForCS_.jsonInfoResource->GetGPUVirtualAddress());

	mainRender->GetCommandList()->Dispatch(UINT(particles[name]->param_["MaxGrains"] + 1023) / 1024, 1, 1);
}

void ParticleManager::DeleteParticle(const std::string& name) {
	// 名前がコンテナ内に存在するかチェック
	auto it = particles.find(name);
	if (it != particles.end()) {
		particles.erase(it);  // コンテナから削除
	}
}

std::string ParticleManager::GenerateName(const std::string& name) {
	// 出力する名前
	std::string outputName = name + "_" + RandomStringUtil::GenerateRandomString(4);

	// 重複チェック用のラムダ式
	std::function<void(const std::string&)> checkDuplicate = [&](const std::string& name) {
		// 重複しているかチェック
		if (particles.find(name) != particles.end()) {
			// 重複しているので名前を変更
			outputName = name + "_" + RandomStringUtil::GenerateRandomString(4);
			checkDuplicate(outputName);
		}
		};

	// 重複チェック
	checkDuplicate(outputName);

	// 最終的に出力
	return outputName;
}

void ParticleManager::GenerateGraphicsPipeline() {
	HRESULT hr;
	auto dxCommon = DirectXCommon::GetInstance();

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//レジスタカウント
	int registerCountVS = 0;
	int registerCountPS = 0;
	//使用するデスクリプタの数
	int numDescriptors = 0;

	// RootParameter作成
	std::vector<D3D12_ROOT_PARAMETER> rootParameters;

	// マテリアル情報用の設定(0)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		param.Descriptor.ShaderRegister = 0;
		rootParameters.push_back(param);
	}
	// 粒配列情報用の設定(1)
	{
		//デスクリプタレンジ作成
		D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
		numDescriptors = 1;
		descriptorRange[0].BaseShaderRegister = registerCountVS;
		descriptorRange[0].NumDescriptors = numDescriptors;
		descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		registerCountVS += numDescriptors;
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		param.DescriptorTable.pDescriptorRanges = descriptorRange;
		param.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
		rootParameters.push_back(param);
	}
	// カメラ情報用の設定(2)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		param.Descriptor.ShaderRegister = 0;
		rootParameters.push_back(param);
	}
	// テクスチャ情報用の設定(3)
	{
		//デスクリプタレンジ作成
		D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
		numDescriptors = 1;
		descriptorRange[0].BaseShaderRegister = registerCountPS;
		descriptorRange[0].NumDescriptors = numDescriptors;
		descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		registerCountPS += numDescriptors;
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		param.DescriptorTable.pDescriptorRanges = descriptorRange;
		param.DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
		rootParameters.push_back(param);
	}
	//エミッター情報用の設定(4)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		param.Descriptor.ShaderRegister = 1;
		rootParameters.push_back(param);
	}
	//JSON情報用の設定(5)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		param.Descriptor.ShaderRegister = 2;
		rootParameters.push_back(param);
	}

	// Sampler作成
	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers(1);
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//Signatureに反映
	descriptionRootSignature.pParameters = rootParameters.data();
	descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters.size());
	descriptionRootSignature.pStaticSamplers = staticSamplers.data();
	descriptionRootSignature.NumStaticSamplers = static_cast<UINT>(staticSamplers.size());

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
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatireBlob->GetBufferPointer(),
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&gRootSignature));
	assert(SUCCEEDED(hr));

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
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendStateの設定
	std::array<D3D12_BLEND_DESC, (int)BlendMode::kMaxBlendModeNum> blendDesc{};
	for (int i = 0; i < (int)BlendMode::kMaxBlendModeNum; i++) {
		//共通部分の設定
		blendDesc[i].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		blendDesc[i].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc[i].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc[i].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

		switch (i) {
		case (int)BlendMode::None:
			blendDesc[i].RenderTarget[0].BlendEnable = FALSE;
			break;
		case (int)BlendMode::Normal:
			blendDesc[i].RenderTarget[0].BlendEnable = TRUE;
			blendDesc[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc[i].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc[i].RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			break;
		case (int)BlendMode::Add:
			blendDesc[i].RenderTarget[0].BlendEnable = TRUE;
			blendDesc[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc[i].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc[i].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;
		case (int)BlendMode::Subtract:
			blendDesc[i].RenderTarget[0].BlendEnable = TRUE;
			blendDesc[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
			blendDesc[i].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc[i].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;
		case (int)BlendMode::Multiply:
			blendDesc[i].RenderTarget[0].BlendEnable = TRUE;
			blendDesc[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc[i].RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
			blendDesc[i].RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
			break;
		case (int)BlendMode::Screen:
			blendDesc[i].RenderTarget[0].BlendEnable = TRUE;
			blendDesc[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc[i].RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
			blendDesc[i].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;
		case (int)BlendMode::Execlution:
			blendDesc[i].RenderTarget[0].BlendEnable = TRUE;
			blendDesc[i].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc[i].RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
			blendDesc[i].RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_COLOR;
			break;
		default:
			break;
		}
	}

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面も表示する
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"Resources/shaders/particle/Particle.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"Resources/shaders/particle/Particle.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	std::array<D3D12_GRAPHICS_PIPELINE_STATE_DESC, (int)BlendMode::kMaxBlendModeNum> graphicsPipelineStateDesc{};
	for (int i = 0; i < (int)BlendMode::kMaxBlendModeNum; i++) {
		graphicsPipelineStateDesc[i].pRootSignature = gRootSignature.Get();
		graphicsPipelineStateDesc[i].InputLayout = inputLayoutDesc;
		graphicsPipelineStateDesc[i].VS = { vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize() };
		graphicsPipelineStateDesc[i].PS = { pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize() };
		graphicsPipelineStateDesc[i].BlendState = blendDesc[i];
		graphicsPipelineStateDesc[i].RasterizerState = rasterizerDesc;
		//書き込むRTVの情報
		graphicsPipelineStateDesc[i].NumRenderTargets = 1;
		graphicsPipelineStateDesc[i].RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		//利用するトポロジのタイプ。三角形
		graphicsPipelineStateDesc[i].PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		//どのように画面に色を打ち込むかの設定
		graphicsPipelineStateDesc[i].SampleDesc.Count = 1;
		graphicsPipelineStateDesc[i].SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		//DepthStencilの設定
		graphicsPipelineStateDesc[i].DepthStencilState = depthStencilDesc;
		graphicsPipelineStateDesc[i].DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		//実際に生成
		hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc[i],
			IID_PPV_ARGS(&graphicsPipelineState[i]));
		assert(SUCCEEDED(hr));
	}
}

void ParticleManager::GenerateComputePipeline() {
	//初期化用CPSOの設定
	InitCPSOOption();
	//エミット用CPSOの設定
	EmitCPSOOption();
	//更新用CPSOの設定
	UpdateCPSOOption();
}

void ParticleManager::InitCPSOOption() {
	HRESULT hr;
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//レジスタカウント
	int registerCountT = 0;		//structuredBuffer専用
	int registerCountU = 0;		//RWStructuredBuffer専用
	//使用するデスクリプタの数
	int numDescriptors = 0;

	//DescriptorRangeの設定
	D3D12_DESCRIPTOR_RANGE descriptorRanges[3] = {};
	// 粒配列用の設定
	numDescriptors = 1;
	descriptorRanges[0].BaseShaderRegister = registerCountU;
	descriptorRanges[0].NumDescriptors = numDescriptors;
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCountU += numDescriptors;
	// フリーリストインデックス用の設定
	numDescriptors = 1;
	descriptorRanges[1].BaseShaderRegister = registerCountU;
	descriptorRanges[1].NumDescriptors = numDescriptors;
	descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCountU += numDescriptors;
	// フリーリスト用の設定
	numDescriptors = 1;
	descriptorRanges[2].BaseShaderRegister = registerCountU;
	descriptorRanges[2].NumDescriptors = numDescriptors;
	descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCountU += numDescriptors;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	//粒配列用の設定(0)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
		param.DescriptorTable.NumDescriptorRanges = 1;
		rootParameters.push_back(param);
	}
	//フリーリストインデックス用の設定(1)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
		param.DescriptorTable.NumDescriptorRanges = 1;
		rootParameters.push_back(param);
	}
	//フリーリスト用の設定(2)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
		param.DescriptorTable.NumDescriptorRanges = 1;
		rootParameters.push_back(param);
	}
	//JSON情報用の設定(3)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.Descriptor.ShaderRegister = 0;
		rootParameters.push_back(param);
	}
	//Signatureに反映
	descriptionRootSignature.pParameters = rootParameters.data();
	descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters.size());

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3D10Blob> signatireBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatireBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとにコンピュートルートシグネチャを生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatireBlob->GetBufferPointer(),
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&cRootSignature[0]));
	assert(SUCCEEDED(hr));

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob;
	computeShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/particle/InitParticle.CS.hlsl",
		L"cs_6_0");
	assert(computeShaderBlob != nullptr);

	//コンピュートシェーダー用のPSOの設定
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
	computePipelineStateDesc.CS = {
		.pShaderBytecode = computeShaderBlob->GetBufferPointer(),
		.BytecodeLength = computeShaderBlob->GetBufferSize()
	};
	computePipelineStateDesc.pRootSignature = cRootSignature[0].Get();
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineState[0]));
}

void ParticleManager::EmitCPSOOption() {
	HRESULT hr;
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//レジスタカウント
	int registerCountT = 0;		//structuredBuffer専用
	int registerCountU = 0;		//RWStructuredBuffer専用
	//使用するデスクリプタの数
	int numDescriptors = 0;

	//DescriptorRangeを作成
	D3D12_DESCRIPTOR_RANGE descriptorRanges[3] = {};
	// 粒配列用の設定
	numDescriptors = 1;
	descriptorRanges[0].BaseShaderRegister = registerCountU;
	descriptorRanges[0].NumDescriptors = numDescriptors;
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCountU += numDescriptors;
	// フリーリストインデックス用の設定
	numDescriptors = 1;
	descriptorRanges[1].BaseShaderRegister = registerCountU;
	descriptorRanges[1].NumDescriptors = numDescriptors;
	descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCountU += numDescriptors;
	// フリーリスト用の設定
	numDescriptors = 1;
	descriptorRanges[2].BaseShaderRegister = registerCountU;
	descriptorRanges[2].NumDescriptors = numDescriptors;
	descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	//粒配列用の設定(0)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
		param.DescriptorTable.NumDescriptorRanges = 1;
		rootParameters.push_back(param);
	}
	//フリーリストインデックス用の設定(1)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
		param.DescriptorTable.NumDescriptorRanges = 1;
		rootParameters.push_back(param);
	}
	//フリーリスト用の設定(2)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
		param.DescriptorTable.NumDescriptorRanges = 1;
		rootParameters.push_back(param);
	}
	//エミッター情報用の設定(3)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.Descriptor.ShaderRegister = 0;
		rootParameters.push_back(param);
	}
	//JSON情報用の設定(4)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.Descriptor.ShaderRegister = 1;
		rootParameters.push_back(param);
	}
	//フレーム情報用の設定(5)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.Descriptor.ShaderRegister = 2;
		rootParameters.push_back(param);
	}
	//Signatureに反映
	descriptionRootSignature.pParameters = rootParameters.data();
	descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters.size());

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3D10Blob> signatireBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatireBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとにコンピュートルートシグネチャを生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatireBlob->GetBufferPointer(),
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&cRootSignature[1]));
	assert(SUCCEEDED(hr));

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob;
	computeShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/particle/EmitParticle.CS.hlsl",
		L"cs_6_0");
	assert(computeShaderBlob != nullptr);

	//コンピュートシェーダー用のPSOの設定
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
	computePipelineStateDesc.CS = {
		.pShaderBytecode = computeShaderBlob->GetBufferPointer(),
		.BytecodeLength = computeShaderBlob->GetBufferSize()
	};
	computePipelineStateDesc.pRootSignature = cRootSignature[1].Get();
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineState[1]));
}

void ParticleManager::UpdateCPSOOption() {
	HRESULT hr;
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//レジスタカウント
	int registerCountT = 0;		//structuredBuffer専用
	int registerCountU = 0;		//RWStructuredBuffer専用
	//使用するデスクリプタの数
	int numDescriptors = 0;

	//DescriptorRangeを作成
	D3D12_DESCRIPTOR_RANGE descriptorRanges[3] = {};
	// 粒配列用の設定
	numDescriptors = 1;
	descriptorRanges[0].BaseShaderRegister = registerCountU;
	descriptorRanges[0].NumDescriptors = numDescriptors;
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCountU += numDescriptors;
	// フリーリストインデックス用の設定
	numDescriptors = 1;
	descriptorRanges[1].BaseShaderRegister = registerCountU;
	descriptorRanges[1].NumDescriptors = numDescriptors;
	descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	registerCountU += numDescriptors;
	// フリーリスト用の設定
	numDescriptors = 1;
	descriptorRanges[2].BaseShaderRegister = registerCountU;
	descriptorRanges[2].NumDescriptors = numDescriptors;
	descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	//粒配列用の設定(0)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
		param.DescriptorTable.NumDescriptorRanges = 1;
		rootParameters.push_back(param);
	}
	//フリーリストインデックス用の設定(1)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
		param.DescriptorTable.NumDescriptorRanges = 1;
		rootParameters.push_back(param);
	}
	//フリーリスト用の設定(2)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
		param.DescriptorTable.NumDescriptorRanges = 1;
		rootParameters.push_back(param);
	}
	//エミッター情報用の設定(3)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.Descriptor.ShaderRegister = 0;
		rootParameters.push_back(param);
	}
	//JSON情報用の設定(4)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.Descriptor.ShaderRegister = 1;
		rootParameters.push_back(param);
	}
	//フレーム情報用の設定(5)
	{
		//ルートパラメータ入力
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		param.Descriptor.ShaderRegister = 2;
		rootParameters.push_back(param);
	}
	//Signatureに反映
	descriptionRootSignature.pParameters = rootParameters.data();
	descriptionRootSignature.NumParameters = static_cast<UINT>(rootParameters.size());

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3D10Blob> signatireBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatireBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとにコンピュートルートシグネチャを生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatireBlob->GetBufferPointer(),
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&cRootSignature[2]));
	assert(SUCCEEDED(hr));

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob;
	computeShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/particle/UpdateParticle.CS.hlsl",
		L"cs_6_0");
	assert(computeShaderBlob != nullptr);

	//コンピュートシェーダー用のPSOの設定
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
	computePipelineStateDesc.CS = {
		.pShaderBytecode = computeShaderBlob->GetBufferPointer(),
		.BytecodeLength = computeShaderBlob->GetBufferSize()
	};
	computePipelineStateDesc.pRootSignature = cRootSignature[2].Get();
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&computePipelineState[2]));
}
