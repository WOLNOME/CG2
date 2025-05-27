#include "CombinedParticle.h"
#include "ParticleManager.h"
#include "JsonUtil.h"
#include <cassert>

void CombinedParticle::Initialize(const std::string& _name) {
	//名前をセット
	name_ = _name;
	//パーティクルのコンテナを初期化
	particles_.clear();
	totalDuration_ = 10.0f;
}

void CombinedParticle::Initialize(const std::string& _name, const std::string& _comParticleFileName) {
	

}

uint32_t CombinedParticle::AddParticle(const std::string& _fileName, float _startTime, float _endTime) {
	//コンテナのサイズが最大数を超えていたらreturn
	if (particles_.size() >= kMaxCombinedParticles) {
		return;
	}
	//新しいパーティクルを作成
	ComParticleInfo newParticle;
	newParticle.startTime = _startTime;
	newParticle.endTime = _endTime;
	newParticle.particle = std::make_unique<Particle>();
	//パーティクルの初期化
	newParticle.particle->Initialize(ParticleManager::GetInstance()->GenerateName(name_), _fileName);
	//パーティクルをコンテナに追加
	particles_[useId_] = std::move(newParticle);
	return useId_++;
}

void CombinedParticle::RemoveParticle(uint32_t _id) {
	//指定されたIDのパーティクルが存在するかチェック
	auto it = particles_.find(_id);
	if (it != particles_.end()) {
		//存在する場合は削除
		particles_.erase(it);
	}
}

const Particle::Emitter& CombinedParticle::GetEmitter(uint32_t _id) {
	//指定されたIDのパーティクルが存在するかチェック
	auto it = particles_.find(_id);
	if (it != particles_.end()) {
		//存在する場合はエミッターを返す
		return it->second.particle->emitter_;
	}
	//存在しない場合はアサート
	assert(0 && "指定されたIDのパーティクルが存在しません。");
	return Particle::Emitter{};
}

void CombinedParticle::SetEmitter(uint32_t _id, const Particle::Emitter& emitter) {
	//指定されたIDのパーティクルが存在するかチェック
	auto it = particles_.find(_id);
	if (it != particles_.end()) {
		//存在する場合はエミッターをセット
		it->second.particle->emitter_ = emitter;
	}
	else {
		//存在しない場合はアサート
		assert(0 && "指定されたIDのパーティクルが存在しません。");
	}
}
