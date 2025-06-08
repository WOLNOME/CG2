#include "Audio.h"
#include "ImGuiManager.h"
#include "StringUtility.h"

Audio::~Audio()
{
	// デストラクタで停止してメモリを解放
	Stop();
}

void Audio::Initialize(const std::string& filename, const std::string& directoryPath)
{
	// ディレクトリパスのコピー
	directoryPath_ = directoryPath;

	// WAVファイルを読み込み
	soundDataHandle_ = AudioCommon::GetInstance()->SoundLoadWave(directoryPath_ + filename);
}

void Audio::DebugWithImGui(const std::wstring& _name) {
#ifdef _DEBUG
	ImGui::Begin("オーディオ");
	if (ImGui::CollapsingHeader(StringUtility::ConvertString(_name).c_str())) {
		if (ImGui::Button("再生")) {
			Play();
		}
		if (ImGui::Button("停止")) {
			Stop();
		}
		if (ImGui::Button("一時停止")) {
			Pause();
		}
		if (ImGui::Button("再開")) {
			Resume();
		}
		ImGui::SliderFloat("音量設定", &volume_, 0.0f, 1.0f);
		SetVolume(volume_);
	}
	ImGui::End();
#endif // _DEBUG
}

void Audio::Play(bool loop, float volume)
{
	// 再生
	voiceDataHandle_ = AudioCommon::GetInstance()->SoundPlayWave(soundDataHandle_, loop, volume);
}

void Audio::Stop()
{
	if (voiceDataHandle_ != 0u) {
		AudioCommon::GetInstance()->SoundStop(voiceDataHandle_);
		voiceDataHandle_ = 0u; // ハンドルを無効化
	}
}

void Audio::Pause()
{
	if (voiceDataHandle_ != 0u) {
		AudioCommon::GetInstance()->SoundPause(voiceDataHandle_);
	}
}

void Audio::Resume()
{
	if (voiceDataHandle_ != 0u) {
		AudioCommon::GetInstance()->SoundResume(voiceDataHandle_);
	}
}

void Audio::SetVolume(float volume)
{
	if (voiceDataHandle_ != 0u) {
		AudioCommon::GetInstance()->SetVolume(voiceDataHandle_, volume);
	}
}
