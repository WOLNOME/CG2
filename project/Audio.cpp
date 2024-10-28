#include "Audio.h"

Audio::~Audio()
{
	//サウンドデータの解放
	audioCommon_->SoundUnload(&soundData_);
}

void Audio::Initialize(AudioCommon* audioCommon, const std::string& filename)
{
	//インスタンス取得
	audioCommon_ = audioCommon;

	//WAVファイル読み込み
	soundData_ = audioCommon_->SoundLoadWave("Resources/" + filename);

}

void Audio::Play()
{
	//再生
	audioCommon_->SoundPlayWave(soundData_);
}
