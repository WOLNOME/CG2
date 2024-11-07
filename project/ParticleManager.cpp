#include "ParticleManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Camera.h"
#include "Logger.h"
#include "Function.h"
#include <cassert>

ParticleManager* ParticleManager::instance = nullptr;

ParticleManager* ParticleManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new ParticleManager;
	}
	return instance;
}

void ParticleManager::Initialize(Camera* camera)
{
	//インスタンス
	dxCommon_ = DirectXCommon::GetInstance();
	srvManager_ = SrvManager::GetInstance();
	camera_ = camera;
	//ランダムエンジンの初期化
	std::random_device seedGenerator;
	randomEngine_.seed(seedGenerator());
	//パイプライン生成
	GenerateGraphicsPipeline();

	//フィールド生成
	accelerationField.acceleration = { -15.0f,0.0f,0.0f };
	accelerationField.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField.area.max = { 1.0f,1.0f,1.0f };
}

void ParticleManager::Update()
{
	//ビルボード行列の計算
	Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
	Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, camera_->GetWorldMatrix());
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//ビュー行列とプロジェクション行列の作成
	Matrix4x4 viewMatrix = camera_->GetViewMatrix();
	Matrix4x4 projectionMatrix = camera_->GetProjectionMatrix();
	//全てのパーティクルグループについて処理
	for (std::unordered_map<std::string, Struct::ParticleGroup>::iterator particleGroupIterator = particleGroups_.begin(); particleGroupIterator != particleGroups_.end();) {
		//パーティクルグループの抽出
		Struct::ParticleGroup* particleGroup = &(particleGroupIterator->second);
		//インスタンスの番号
		particleGroup->instanceNum = 0;
		//全てのパーティクルについての処理
		for (std::list<Struct::Particle>::iterator particleIterator = particleGroup->particles.begin(); particleIterator != particleGroup->particles.end();) {
			//寿命に達していたらグループから外す
			if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup->particles.erase(particleIterator);
				continue;
			}
			//場の影響を計算
			if (IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
				(*particleIterator).velocity = Add((*particleIterator).velocity, Multiply(kDeltaTime, accelerationField.acceleration));
			}
			//移動処理
			(*particleIterator).transform.translate = Add((*particleIterator).transform.translate, Multiply(kDeltaTime, (*particleIterator).velocity));
			//α値設定
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
			//経過時間を加算
			(*particleIterator).currentTime++;
			//ワールド行列を計算
			Matrix4x4 worldMatrix = Multiply(Multiply(MakeScaleMatrix((*particleIterator).transform.scale), billboardMatrix), MakeTranslateMatrix((*particleIterator).transform.translate));
			//ワールドビュープロジェクション行列を合成
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			//インスタンシング用データの書き込み
			particleGroup->instancingData[particleGroup->instanceNum].World = worldMatrix;
			particleGroup->instancingData[particleGroup->instanceNum].WVP = worldViewProjectionMatrix;
			particleGroup->instancingData[particleGroup->instanceNum].color.x = (*particleIterator).color.x;
			particleGroup->instancingData[particleGroup->instanceNum].color.y = (*particleIterator).color.y;
			particleGroup->instancingData[particleGroup->instanceNum].color.z = (*particleIterator).color.z;
			particleGroup->instancingData[particleGroup->instanceNum].color.w = alpha;

			//インスタンス数のインクリメント
			particleGroup->instanceNum++;
			//イテレーターのインクリメント
			particleIterator++;
		};
		//モデル（見た目）の更新
		particleGroup->model->Update();
		//イテレーターのインクリメント
		particleGroupIterator++;
	}
}

void ParticleManager::Draw()
{
	// ルートシグネチャの設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
	// パイプラインの設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());
	// 描画
	for (std::unordered_map<std::string, Struct::ParticleGroup>::iterator particleGroupIterator = particleGroups_.begin(); particleGroupIterator != particleGroups_.end();) {
		//座標変換行列Tableの場所を設定
		dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUDescriptorHandle((*particleGroupIterator).second.instancingSrvIndex));
		//モデルの描画
		(*particleGroupIterator).second.model->Draw((*particleGroupIterator).second.materialData.textureFilePath,(*particleGroupIterator).second.particles.size());

		++particleGroupIterator;
	}

}

void ParticleManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void ParticleManager::Emit(const std::string name, const Vector3& position, uint32_t count)
{
	//読み込み済みチェック
	if (!particleGroups_.contains(name)) {
		//nameキーが読み込んでいないのでreturn;
		return;
	}
	//countの数だけ登録
	for (uint32_t index = 0; index < count; ++index) {
		particleGroups_.find(name)->second.particles.push_back(MakeNewParticle(position));
	}
}

void ParticleManager::CreateParticleGroup(const std::string& name, const std::string& modelFilePath)
{
	//重複防止処理
	if (particleGroups_.contains(name)) {
		//読み込み済みなら早期リターン
		return;
	}
	//新たな空のパーティクルグループを作成
	Struct::ParticleGroup particleGroup;
	//モデル生成
	ModelManager::GetInstance()->LoadModel(modelFilePath);
	//モデル登録
	particleGroup.model = ModelManager::GetInstance()->FindModel(modelFilePath);
	//カメラのセット
	particleGroup.model->SetCamera(camera_);
	//マテリアルデータ設定(0番目のモデルデータのマテリアルを参照)
	particleGroup.materialData.textureFilePath = "";
	//インスタンシング用リソースの生成
	particleGroup.instancingResource = dxCommon_->CreateBufferResource(sizeof(Struct::ParticleForGPU) * kNumMaxInstance_);
	particleGroup.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroup.instancingData));
	//インスタンシング用にSRVを確保してSRVインデックスを記録
	uint32_t index = srvManager_->Allocate();
	particleGroup.instancingSrvIndex = index;
	//SRV生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = kNumMaxInstance_;
	srvDesc.Buffer.StructureByteStride = sizeof(Struct::ParticleForGPU);
	dxCommon_->GetDevice()->CreateShaderResourceView(particleGroup.instancingResource.Get(), &srvDesc, srvManager_->GetCPUDescriptorHandle(particleGroup.instancingSrvIndex));
	//新たに作ったパーティクルグループを登録
	particleGroups_[name] = particleGroup;
}

void ParticleManager::SetTexture(const std::string& name, const std::string& textureFilePath)
{
	//読み込み済みチェック
	if (!particleGroups_.contains(name)) {
		//nameキーが読み込んでいないのでreturn;
		return;
	}
	//テクスチャのセット
	particleGroups_.find(name)->second.materialData.textureFilePath = textureFilePath;

}

void ParticleManager::GenerateGraphicsPipeline()
{
	HRESULT hr;

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;
	descriptorRangeForInstancing[0].NumDescriptors = 1;
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	//マテリアルの設定
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号0とバインド
	//オブジェクト関連の設定
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Tableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);
	//テクスチャの設定
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Tableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);
	//平行光源用の設定
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;//レジスタ番号1とバインド

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
	D3D12_BLEND_DESC blendDesc{};
	//全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	// アルファブレンド設定
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Particle.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/Particle.PS.hlsl",
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

ParticleManager::Struct::Particle ParticleManager::MakeNewParticle(const Vector3& translate)
{
	Struct::Particle particle;
	//トランスフォーム
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	Vector3 randomTranslate = { distribution(randomEngine_),distribution(randomEngine_) ,distribution(randomEngine_) };
	particle.transform.translate = Add(translate, randomTranslate);
	//速度
	particle.velocity = { distribution(randomEngine_) ,distribution(randomEngine_) ,distribution(randomEngine_) };
	//色
	std::uniform_real_distribution<float> distcolor(0.0f, 1.0f);
	particle.color = { distcolor(randomEngine_) ,distcolor(randomEngine_) ,distcolor(randomEngine_),1.0f };
	//寿命
	std::uniform_real_distribution<float> distTime(1.0f * 60.0f, 3.0f * 60.0f);
	particle.lifeTime = distTime(randomEngine_);
	particle.currentTime = 0;

	return particle;
}
