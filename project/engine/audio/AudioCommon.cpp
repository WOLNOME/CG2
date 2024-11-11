#include "AudioCommon.h"
#include <cassert>
#include <fstream>

AudioCommon* AudioCommon::instance = nullptr;

AudioCommon* AudioCommon::GetInstance()
{
	if (instance == nullptr) {
		instance = new AudioCommon;
	}
	return instance;
}

void AudioCommon::Initialize()
{
	//XAudio2エンジンのインスタンス作成
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	//マスターボイスを作成
	hr = xAudio2_->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));

}

void AudioCommon::Finalize()
{
	//XAudio2の解放
	xAudio2_.Reset();
	delete instance;
	instance = nullptr;
}

AudioCommon::SoundData AudioCommon::SoundLoadWave(const std::string& filename)
{
	// 1. 既に読み込まれているかチェック
	auto it = soundDatas_.find(filename);
	if (it != soundDatas_.end()) {
		// キャッシュからデータを返す
		return it->second;
	}

	// 2. ファイルオープン
	std::ifstream file;
	file.open("Resources/audios/" + filename, std::ios_base::binary);
	assert(file.is_open());

	// 3. 「.wav」データ読み込み
	RiffHeader riff;
	file.read(reinterpret_cast<char*>(&riff), sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	FormatChunk format = {};
	file.read(reinterpret_cast<char*>(&format), sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read(reinterpret_cast<char*>(&format.fmt), format.chunk.size);

	ChunkHeader data;
	file.read(reinterpret_cast<char*>(&data), sizeof(data));
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read(reinterpret_cast<char*>(&data), sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// 4. バッファにデータを読み込む
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// 5. ファイルクローズ
	file.close();

	// 6. 読み込んだ音声データを作成
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	// 7. 作成した音声データをキャッシュにほぞん
	soundDatas_[filename] = soundData;

	// 8. 読み込んだ音声データを返す
	return soundData;
}

IXAudio2SourceVoice* AudioCommon::GenerateSourceVoice(SoundData& soundData)
{
	// ソースボイスがすでに存在しない場合は新しく作成
	IXAudio2SourceVoice* sourceVoice = nullptr;
	HRESULT result = xAudio2_->CreateSourceVoice(&sourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));
	return sourceVoice;
}

void AudioCommon::SoundUnload(SoundData* soundData)
{
	// バッファのメモリを解放
	delete[] soundData->pBuffer;
	soundData->pBuffer = nullptr;

	// 構造体の初期化
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void AudioCommon::SoundPlayWave(SoundData& soundData, IXAudio2SourceVoice* sourceVoice, bool loop)
{
	HRESULT result;

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

	// 波形データの再生
	sourceVoice->Stop();
	sourceVoice->FlushSourceBuffers();
	result = sourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));
	result = sourceVoice->Start();
	assert(SUCCEEDED(result));

}

void AudioCommon::SoundPause(IXAudio2SourceVoice* sourceVoice)
{
	if (sourceVoice) {
		sourceVoice->Stop();
	}
}

void AudioCommon::SoundResume(IXAudio2SourceVoice* sourceVoice)
{
	if (sourceVoice) {
		sourceVoice->Start();
	}
}

void AudioCommon::SoundStop(IXAudio2SourceVoice* sourceVoice)
{
	if (sourceVoice) {
		sourceVoice->Stop();
		sourceVoice->FlushSourceBuffers();
	}
}

void AudioCommon::SetVolume(IXAudio2SourceVoice* sourceVoice, float volume)
{
	if (sourceVoice) {
		sourceVoice->SetVolume(volume);
	}
}

void AudioCommon::SetLoop(SoundData& soundData, IXAudio2SourceVoice* sourceVoice, bool loop)
{
	if (sourceVoice) {
		XAUDIO2_BUFFER buf{};
		buf.pAudioData = soundData.pBuffer;
		buf.AudioBytes = soundData.bufferSize;
		buf.Flags = XAUDIO2_END_OF_STREAM;
		buf.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
		sourceVoice->Stop();
		sourceVoice->FlushSourceBuffers();
		sourceVoice->SubmitSourceBuffer(&buf);
	}
}

void AudioCommon::ClearSoundDatas()
{
	for (auto& pair : soundDatas_) {
		SoundUnload(&pair.second);
	}
	soundDatas_.clear();
}
