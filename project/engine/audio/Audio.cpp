#include "Audio.h"

Audio::~Audio()
{
	//サウンドデータの解放
	AudioCommon::GetInstance()->SoundUnload(&soundData_);

	// ソースボイスの破棄
	if (sourceVoice_) {
		sourceVoice_->Stop();
		sourceVoice_->FlushSourceBuffers();
		sourceVoice_->DestroyVoice();
		sourceVoice_ = nullptr;
	}
}

void Audio::Initialize(const std::string& filename)
{
	//WAVファイル読み込み
	soundData_ = AudioCommon::GetInstance()->SoundLoadWave(filename);

	//ソースボイスを生成
	sourceVoice_ = AudioCommon::GetInstance()->GenerateSourceVoice(soundData_);
}

void Audio::Play(bool loop)
{
	//再生
	AudioCommon::GetInstance()->SoundPlayWave(soundData_, sourceVoice_, loop);
}

void Audio::Stop()
{
	AudioCommon::GetInstance()->SoundStop(sourceVoice_);
	isPlaying_ = false;
	isPaused_ = false;
}

void Audio::Pause()
{
	if (isPlaying_ && !isPaused_) {
		AudioCommon::GetInstance()->SoundPause(sourceVoice_);
		isPaused_ = true;
	}
}

void Audio::Resume()
{
	if (isPaused_) {
		AudioCommon::GetInstance()->SoundResume(sourceVoice_);
		isPaused_ = false;
	}
}

bool Audio::IsPlaying() const
{
	// 状態を直接返す
	return isPlaying_ && !isPaused_;
}

bool Audio::IsPaused() const
{
	return isPaused_;
}

void Audio::SetVolume(float volume)
{
	volume_ = volume;
	AudioCommon::GetInstance()->SetVolume(sourceVoice_, volume);
}

float Audio::GetVolume() const
{
	return volume_;
}

void Audio::SetPosition(const Vector3& v)
{
	// Position設定処理
	// AudioCommonに専用関数を追加する場合も検討
}

void Audio::SetListenerPosition(const Vector3& v)
{
	// リスナー位置の更新
   // AudioCommonでリスナー情報を管理する場合
}

void Audio::SetOnPlaybackEndCallback(std::function<void()> callback)
{
	onPlaybackEndCallback_ = callback;
}
