#include "Particle.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "ModelManager.h"
#include "ParticleManager.h"
#include "JsonUtil.h"

Particle::~Particle() {
	//確保したSRVデスクリプタヒープの解放
	SrvManager::GetInstance()->Free(particleResource_.srvIndex);
	//マネージャーから削除
	ParticleManager::GetInstance()->DeleteParticle(name_);
}

void Particle::Initialize(const std::string& name, const std::string& fileName) {
	//名前を登録
	name_ = name;
	//パラメータをセット
	auto data = JsonUtil::GetJsonData("Resources/particles/" + fileName);
	//JSONファイルの読み込み
	if (data) param_ = data;
	else assert(0 && "JSONファイルが存在しません");
	//エミッターの初期化
	emitter_.transform.translate = Vector3(0.0f, 0.0f, 0.0f);
	emitter_.transform.rotate = Vector3(0.0f, 0.0f, 0.0f);
	emitter_.transform.scale = Vector3(1.0f, 1.0f, 1.0f);
	emitter_.generateMethod = GenerateMethod::kRandom;
	emitter_.gravity = -1.0f;
	emitter_.repulsion = 0.5f;
	emitter_.floorHeight = 0.0f;
	emitter_.clumpNum = 1;
	emitter_.isAffectedField = false;
	emitter_.isBillboard = true;
	emitter_.isGravity = false;
	emitter_.isBound = false;
	emitter_.isPlay = true;

	//形状を生成
	shape_ = std::make_unique<Shape>();
	//形状の初期化
	Shape::ShapeKind shapeKind = (Shape::ShapeKind)param_["Primitive"];
	shape_->Initialize(shapeKind);

	//パーティクルのリソースを作成
	particleResource_ = MakeParticleResource();
	//インスタンシングをSRVにセット
	SettingSRV();

	//最後にマネージャーに登録
	ParticleManager::GetInstance()->RegisterParticle(name_, this);
}

Particle::ParticleResource Particle::MakeParticleResource() {
	//Particleリソース
	ParticleResource particleResource;
	//インスタンシングリソース作成
	uint32_t kNumMaxInstance = param_["MaxGrains"];
	particleResource.instancingResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);
	//リソースにデータを書き込む
	particleResource.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleResource.instancingData));
	//データに書き込む
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		particleResource.instancingData[index].World = MyMath::MakeIdentity4x4();
		particleResource.instancingData[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	//リターン
	return particleResource;
}

void Particle::SettingSRV() {
	//SRVマネージャーからデスクリプタヒープの空き番号を取得
	particleResource_.srvIndex = SrvManager::GetInstance()->Allocate();

	//srv設定
	uint32_t kNumMaxInstance = param_["MaxGrains"];
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = kNumMaxInstance;
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(particleResource_.instancingResource.Get(), &srvDesc, SrvManager::GetInstance()->GetCPUDescriptorHandle(particleResource_.srvIndex));
}

void Particle::ShapeChange() {
	//JSONをもとにShapeを作り直す
	shape_ = std::make_unique<Shape>();
	//形状の初期化
	Shape::ShapeKind shapeKind = (Shape::ShapeKind)param_["Primitive"];
	shape_->Initialize(shapeKind);
}
