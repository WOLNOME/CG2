#pragma once
#include "Particle.h"
#include <string>
#include <cstdint>
#include <unordered_map>
#include <memory>
#include <queue>

class CombinedParticle {
	//パーティクルマネージャーに公開
	friend class ParticleManager;
	//パーティクルクリエイターシーンに公開
	friend class ParticleCreatorScene;
public://構造体
	struct ComParticleInfo {
		std::unique_ptr<Particle> particle; //パーティクル本体
		float startTime = 0.0f; //発生開始時間(0~1)
		float endTime = 0.0f; //発生終了時間(0~1)
		bool isDislplayed = true; //表示フラグ
	};

public:
	void Initialize(const std::string& _name);
	void Initialize(const std::string& _name, const std::string& _comParticleFileName);

	//新しいパーティクルを追加
	uint32_t AddParticle(const std::string& _fileName, float _startTime, float _endTime);
	//パーティクルを削除
	void RemoveParticle(uint32_t _id);

public://getter
	//エミッターを取得
	const Particle::Emitter& GetEmitter(uint32_t _id);

public://setter
	//エミッターをセット
	void SetEmitter(uint32_t _id, const Particle::Emitter& emitter);

private:
	//名前
	std::string name_;

	//パーティクルのコンテナ
	std::unordered_map<uint32_t, ComParticleInfo> particles_;

	//全体の尺
	float totalDuration_ = 0.0f;

	//複合最大数
	const uint32_t kMaxCombinedParticles = 8;

	//最新のID
	uint32_t useId_ = 0;

};

