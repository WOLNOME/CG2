#include "Audio.h"

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

void Audio::Play()
{
    // 再生
    voiceDataHandle_ = AudioCommon::GetInstance()->SoundPlayWave(soundDataHandle_, false, 0.5f);
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
