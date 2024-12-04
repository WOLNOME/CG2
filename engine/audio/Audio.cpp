#include "Audio.h"

Audio::~Audio()
{
	//サウンドデータの解放
	AudioCommon::GetInstance()->SoundUnload(&soundData_);
}

void Audio::Initialize(const std::string& filename)
{
	//WAVファイル読み込み
	soundData_ = AudioCommon::GetInstance()->SoundLoadWave("Resources/" + filename);

}

void Audio::Play()
{
	//再生
	AudioCommon::GetInstance()->SoundPlayWave(soundData_);
}
