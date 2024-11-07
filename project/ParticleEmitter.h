#pragma once
#include "ParticleManager.h"
#include "Function.h"
#include <cstdint>
#include <string>

class ParticleEmitter
{
private:
	class Struct {
	public:
		struct Emitter {
			Transform transform; // エミッタのTransform
			uint32_t count; // 発生数
			float frequency; // 発生頻度
			float frequencyTime; // 頻度用時刻
		};
	};
public:
	//初期化
	void Initialize(const std::string& name);
	//更新
	void Update();
	//エミット
	void Emit();
private:
	Struct::Emitter emitter_;
	std::string name_;
	//δt
	const float kDeltaTime = 1.0f / 60.0f;
};