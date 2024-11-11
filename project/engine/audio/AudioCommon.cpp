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
	assert(strncmp(riff.chunk.id, "RIFF", 4) == 0);
	assert(strncmp(riff.type, "WAVE", 4) == 0);

	FormatChunk format = {};
	file.read(reinterpret_cast<char*>(&format), sizeof(ChunkHeader));
	assert(strncmp(format.chunk.id, "fmt ", 4) == 0);
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read(reinterpret_cast<char*>(&format.fmt), format.chunk.size);

	ChunkHeader data;
	file.read(reinterpret_cast<char*>(&data), sizeof(data));
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read(reinterpret_cast<char*>(&data), sizeof(data));
	}
	assert(strncmp(data.id, "data", 4) == 0);

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

	// 7. ソースボイスの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	HRESULT hr = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(hr)); // エラーチェック
	soundData.sourceVoice.reset(pSourceVoice, [](IXAudio2SourceVoice* voice) {
		if (voice) {
			voice->DestroyVoice(); // リソース解放
		}
		});

	// バッファをソースボイスに送信
	XAUDIO2_BUFFER xaudioBuffer = {};
	xaudioBuffer.AudioBytes = soundData.bufferSize;
	xaudioBuffer.pAudioData = soundData.pBuffer;
	xaudioBuffer.Flags = XAUDIO2_END_OF_STREAM; // 再生終了を通知
	hr = soundData.sourceVoice->SubmitSourceBuffer(&xaudioBuffer);
	assert(SUCCEEDED(hr));

	// 8. 作成した音声データをキャッシュに保存
	soundDatas_[filename] = soundData;

	// 9. 読み込んだ音声データを返す
	return soundData;
}

void AudioCommon::SoundPlayWave(uint32_t soundDataHandle, bool loop)
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

void AudioCommon::SoundPause(uint32_t voiceDataHandle)
{
	if (sourceVoice) {
		sourceVoice->Stop();
	}
}

void AudioCommon::SoundResume(uint32_t voiceDataHandle)
{
	if (sourceVoice) {
		sourceVoice->Start();
	}
}

void AudioCommon::SoundStop(uint32_t voiceDataHandle)
{
	if (sourceVoice) {
		sourceVoice->Stop();
		sourceVoice->FlushSourceBuffers();
	}
}

void AudioCommon::SetVolume(uint32_t voiceDataHandle, float volume)
{
	if (sourceVoice) {
		sourceVoice->SetVolume(volume);
	}
}

void AudioCommon::SetLoop(uint32_t soundDataHandle, uint32_t voiceDataHandle, bool loop)
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
