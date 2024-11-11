#include "Audio.h"

Audio::~Audio()
{
	//サウンドデータの解放
	AudioCommon::GetInstance()->SoundUnload(&soundData_);
}

void Audio::Initialize(const std::string& filename)
{
	//WAVファイル読み込み
	soundData_ = AudioCommon::GetInstance()->SoundLoadWave(filename);

}

void Audio::Play(bool loop = false)
{
	//再生
	AudioCommon::GetInstance()->SoundPlayWave(soundData_, loop);
}

void Audio::Stop()
{
	AudioCommon::GetInstance()->SoundStop(soundData_);
}

void Audio::Pause()
{
	AudioCommon::GetInstance()->SoundPause(soundData_);
}

void Audio::Resume()
{
	AudioCommon::GetInstance()->SoundResume(soundData_);
}

bool Audio::IsPlaying() const
{
	return false;
}

bool Audio::IsPaused() const
{
	return false;
}

void Audio::SetVolume(float volume)
{
	AudioCommon::GetInstance()->SetVolume(soundData_, volume);
}

float Audio::GetVolume() const
{
	return 0.0f;
}

void Audio::FadeIn(float duration, float min, float max)
{
}

void Audio::FadeOut(float duration, float min, float max)
{
}

void Audio::SetPosition(const Vector3& v)
{
}

void Audio::SetListenerPosition(const Vector3& v)
{
}

void Audio::SetOnPlaybackEndCallback(std::function<void()> callback)
{
}
