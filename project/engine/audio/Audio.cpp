#include "Audio.h"
#include <cassert>
#include <fstream>

Audio* Audio::instance = nullptr;

Audio* Audio::GetInstance()
{
	if (instance == nullptr) {
		instance = new Audio;
	}
	return instance;
}

void Audio::Initialize()
{
	//XAudio2エンジンのインスタンス作成
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	//マスターボイスを作成
	hr = xAudio2_->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));
}

void Audio::Finalize()
{
	//サウンドデータの解放
	Unload();
	//xAudioの解放
	xAudio2_.Reset();
	//インスタンスの解放
	delete instance;
	instance = nullptr;
}

void Audio::Play()
{
	HRESULT result;
	//波形フォーマットからSourceVoiceを生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData_.wfex);
	assert(SUCCEEDED(result));
	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData_.pBuffer;
	buf.AudioBytes = soundData_.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	//波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

void Audio::LoadWave(const std::string& filename)
{
	//1.ファイルオープン
	std::ifstream file;
	file.open("Resources/" + filename, std::ios_base::binary);
	assert(file.is_open());
	//2.「.wav」データ読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}
	FormatChunk format = {};
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	//3.ファイルクローズ
	file.close();
	//4.読み込んだ音声データをreturn
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	//サウンドデータ入力
	soundData_ = soundData;
}

void Audio::Unload()
{
	//バッファのメモリを解放
	delete[] soundData_.pBuffer;
	soundData_.pBuffer = 0;
	soundData_.bufferSize = 0;
	soundData_.wfex = {};
}
