#include "Particle.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleCommon.h"
#include "ImGuiManager.h"
#include "BaseCamera.h"
#include <fstream>
#include <sstream>
#include <random>

Particle::~Particle()
{
	//確保したSRVデスクリプタヒープの解放
	SrvManager::GetInstance()->Free(particleResource_.srvIndex);
}

void Particle::Initialize(const std::string& filePath)
{
	//モデルマネージャーでモデル(見た目)を生成
	ModelManager::GetInstance()->LoadModel(filePath);
	//モデルマネージャーから検索してセットする
	model_ = ModelManager::GetInstance()->FindModel(filePath);

	//パーティクルのリソースを作成
	particleResource_ = MakeParticleResource();
	//インスタンシングをSRVにセット
	SettingSRV();

	//エミッター生成
	emitter.transform.scale = { 1.0f,1.0f,1.0f };
	emitter.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter.transform.translate = { 0.0f,0.0f,0.0f };
	emitter.count = 3;//1度に3個生成する
	emitter.frequency = 0.5f;//0.5秒ごとに発生
	emitter.frequencyTime = 0.0f;//currentTime

	//フィールド生成
	accelerationField.acceleration = { 15.0f,0.0f,0.0f };
	accelerationField.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField.area.max = { 1.0f,1.0f,1.0f };

}

void Particle::Update()
{
#ifdef _DEBUG
	ImGui::Begin("particle");
	ImGui::Checkbox("billboard", &isBillboard);
	ImGui::Checkbox("field", &isField);
	if (ImGui::Button("Add Particle")) {
		//パーティクル生成
		particles.splice(particles.end(), Emit(emitter));
	}
	ImGui::DragFloat3("EmitterTranslate", &emitter.transform.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::End();
#endif // _DEBUG
}

void Particle::Draw(const BaseCamera& camera)
{
	//インスタンスの番号
	uint32_t instanceNum = 0;
	//エミッターの更新
	emitter.frequencyTime += kDeltaTime;
	if (emitter.frequency <= emitter.frequencyTime) {
		particles.splice(particles.end(), Emit(emitter));
		emitter.frequencyTime -= emitter.frequency;
	}

	for (std::list<Struct::Particle>::iterator particleIterator = particles.begin(); particleIterator != particles.end();) {
		//時間更新
		++(*particleIterator).currentTime;

		//生存チェック
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			//寿命を迎えたら削除
			particleIterator = particles.erase(particleIterator);
			continue;
		}

		//フィールドの処理
		if (isField) {
			if (MyMath::IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
				(*particleIterator).velocity = MyMath::Add((*particleIterator).velocity, MyMath::Multiply(kDeltaTime, accelerationField.acceleration));
			}
		}

		//速度加算処理
		(*particleIterator).transform.translate = MyMath::Add((*particleIterator).transform.translate, MyMath::Multiply(kDeltaTime, (*particleIterator).velocity));
		//α値設定
		float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);

		//レンダリングパイプライン
		Matrix4x4 backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);
		Matrix4x4 billboardMatrix = MyMath::Multiply(backToFrontMatrix, camera.GetWorldMatrix());
		billboardMatrix.m[3][0] = 0.0f;
		billboardMatrix.m[3][1] = 0.0f;
		billboardMatrix.m[3][2] = 0.0f;
		Matrix4x4 worldMatrix = MyMath::Multiply(MyMath::Multiply(MyMath::MakeScaleMatrix((*particleIterator).transform.scale), billboardMatrix), MyMath::MakeTranslateMatrix((*particleIterator).transform.translate));
		if (!isBillboard) {
			worldMatrix = MyMath::MakeAffineMatrix((*particleIterator).transform.scale, (*particleIterator).transform.rotate, (*particleIterator).transform.translate);
		}
		particleResource_.instancingData[instanceNum].World = worldMatrix;
		particleResource_.instancingData[instanceNum].color = (*particleIterator).color;
		particleResource_.instancingData[instanceNum].color.w = alpha;

		//モデルの更新
		model_->Update();

		//インスタンスの数インクリメント
		++instanceNum;
		//次のイテレータに進める
		++particleIterator;
	}

	//座標変換行列Tableの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(1, particleResource_.SrvHandleGPU);
	//CameraCBufferの場所を設定
	DirectXCommon::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());
	//モデルの描画
	model_->Draw(0, 3, (uint32_t)particles.size());

}

Particle::Struct::ParticleResource Particle::MakeParticleResource()
{
	//Particleリソース
	Struct::ParticleResource particleResource;
	//インスタンシングリソース作成
	particleResource.instancingResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Struct::ParticleForGPU) * kNumMaxInstance_);
	//リソースにデータを書き込む
	particleResource.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleResource.instancingData));
	//データに書き込む
	for (uint32_t index = 0; index < kNumMaxInstance_; ++index) {
		particleResource.instancingData[index].World = MyMath::MakeIdentity4x4();
		particleResource.instancingData[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	//トランスフォーム
	particleResource.transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};
	//リターン
	return particleResource;
}

void Particle::SettingSRV()
{
	//SRVマネージャーからデスクリプタヒープの空き番号を取得
	particleResource_.srvIndex = SrvManager::GetInstance()->Allocate();

	//srv設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = kNumMaxInstance_;
	srvDesc.Buffer.StructureByteStride = sizeof(Struct::ParticleForGPU);
	particleResource_.SrvHandleCPU = SrvManager::GetInstance()->GetCPUDescriptorHandle(particleResource_.srvIndex);
	particleResource_.SrvHandleGPU = SrvManager::GetInstance()->GetGPUDescriptorHandle(particleResource_.srvIndex);
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(particleResource_.instancingResource.Get(), &srvDesc, particleResource_.SrvHandleCPU);
}

Particle::Struct::Particle Particle::MakeNewParticle(const Vector3& translate)
{
	Struct::Particle particle;
	//ランダムエンジンの生成
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());
	//トランスフォーム
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	Vector3 randomTranslate = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.transform.translate = MyMath::Add(translate, randomTranslate);
	//速度
	particle.velocity = { distribution(randomEngine) ,distribution(randomEngine) ,distribution(randomEngine) };
	//色
	std::uniform_real_distribution<float> distcolor(0.0f, 1.0f);
	particle.color = { distcolor(randomEngine) ,distcolor(randomEngine) ,distcolor(randomEngine),1.0f };
	//寿命
	std::uniform_real_distribution<float> distTime(1.0f * 60.0f, 3.0f * 60.0f);
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

std::list<Particle::Struct::Particle> Particle::Emit(const Struct::Emitter& emitter)
{
	std::list<Struct::Particle> particle;

	for (uint32_t count = 0; count < emitter.count; ++count) {
		particle.push_back(MakeNewParticle(emitter.transform.translate));
	}

	return particle;
}