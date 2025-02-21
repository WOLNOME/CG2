#include "ParticleEmitter.h"
#include "ImGuiManager.h"

void ParticleEmitter::Initialize(const std::string& name)
{
	//キーを保存
	name_ = name;

	//エミッター生成
	emitter_.transform.scale = { 1.0f,1.0f,1.0f };
	emitter_.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter_.transform.translate = { 0.0f,0.0f,0.0f };
	emitter_.count = 3;//1度に3個生成する
	emitter_.frequency = 0.5f;//0.5秒ごとに発生
	emitter_.frequencyTime = 0.0f;//currentTime
}

void ParticleEmitter::Update()
{
	//エミッターの更新
	emitter_.frequencyTime += kDeltaTime;
	//エミッターが発生
	if (emitter_.frequency <= emitter_.frequencyTime) {
		//エミット
		Emit();
		//エミッターの現在時間をエミッターの発生頻度で引く
		emitter_.frequencyTime -= emitter_.frequency;
	}

#ifdef _DEBUG
	//自己発生
	ImGui::Begin(name_.c_str());
	if (ImGui::Button("Emit")) {
		Emit();
	}
	ImGui::End();
#endif // _DEBUG

}

void ParticleEmitter::Emit()
{
	ParticleManager::GetInstance()->Emit(name_, emitter_.transform.translate, emitter_.count);
}
