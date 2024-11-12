#include "AudioCommon.h"
#include <cassert>
#include <fstream>

AudioCommon* AudioCommon::instance = nullptr;

//サウンドデータコンテナの開始位置
const uint32_t kStartSoundDataIndex = 1;

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
	//コンテナの全開放
	ShutdownContainer();
	//XAudio2の解放
	xAudio2_.Reset();
	delete instance;
	instance = nullptr;
}


uint32_t AudioCommon::SoundLoadWave(const std::string& filename)
{
	// 1. 既存のサウンドデータを検索
	for (uint32_t i = kStartSoundDataIndex; i < soundDatas_.size(); ++i) {
		if (soundDatas_[i].name == filename) {  // 名前が一致するサウンドデータを発見
			return i;                           // 既存のインデックスを返す
		}
	}

	// 2. ファイルオープン
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	assert(file.is_open());

	// 3. 「.wav」データ読み込み
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

	// 4. ファイルクローズ
	file.close();

	// 5. サウンドデータの登録
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;
	soundData.name = filename;  // ファイル名を保存

	// soundDatas_に空きがある場所を検索して登録
	for (uint32_t i = kStartSoundDataIndex; i < soundDatas_.size(); ++i) {
		if (soundDatas_[i].pBuffer == nullptr) { // 空きスロットを検索
			soundDatas_[i] = soundData;          // 新しいデータを格納
			return i;                            // インデックスを返す
		}
	}

	// 空きがない場合はエラー
	delete[] pBuffer;
	assert(false && "No available space in soundDatas_");
	return -1; // ここには到達しないが、警告回避のため
}

uint32_t AudioCommon::SoundPlayWave(uint32_t soundDataHandle, bool loop, float volume)
{
	//手順
	//空のボイスデータを作成
	//ボイスデータコンテナからボイスデータ内のhandleと引数のサウンドデータハンドルが一致しているか確認
	//一致してる→空のボイスデータに既存のボイスデータのポインタを移植
	//一致してない→新しくボイスデータを作る→コンテナに登録。
	//ボイスデータの準備が両方できたので再生等各種設定


	assert(soundDataHandle < soundDatas_.size()); // ハンドルの範囲チェック
	SoundData& soundData = soundDatas_[soundDataHandle];
	assert(soundData.pBuffer != nullptr); // 有効なサウンドデータが存在することを確認

	// 1. voiceDatas_から既存のボイスデータを検索
	VoiceData* targetVoiceData = nullptr;
	for (VoiceData* voiceData : voiceDatas_) {
		if (voiceData->handle == soundDataHandle) {
			targetVoiceData = voiceData; // 一致するボイスデータを保持
			break;
		}
	}

	// 2. 一致するボイスデータがない場合、新しいボイスデータを作成
	if (targetVoiceData == nullptr) {
		targetVoiceData = new VoiceData();
		targetVoiceData->handle = soundDataHandle;

		HRESULT result = xAudio2_->CreateSourceVoice(&targetVoiceData->sourceVoice, &soundData.wfex);
		assert(SUCCEEDED(result));

		voiceDatas_.insert(targetVoiceData); // 新しいボイスデータをコンテナに登録
	}

	assert(targetVoiceData->sourceVoice != nullptr); // ソースボイスが有効であることを確認

	// 3. 波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	if (loop) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE; // 無限ループ
	}
	else {
		buf.LoopCount = 0; // ループなし
	}

	// 4. サウンドの再生準備
	HRESULT result = targetVoiceData->sourceVoice->Stop();
	result = targetVoiceData->sourceVoice->FlushSourceBuffers();
	result = targetVoiceData->sourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));

	// 5. 音量の設定
	result = targetVoiceData->sourceVoice->SetVolume(volume);
	assert(SUCCEEDED(result));

	// 6. サウンドの再生
	result = targetVoiceData->sourceVoice->Start();
	assert(SUCCEEDED(result));

	// 7. 該当するボイスデータの位置を返す
	return soundDataHandle;
}

void AudioCommon::SoundUnload(SoundData* soundData)
{
	//バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void AudioCommon::SoundStop(uint32_t voiceHandle)
{
	auto it = std::find_if(voiceDatas_.begin(), voiceDatas_.end(),
		[voiceHandle](VoiceData* data) { return data->handle == voiceHandle; });

	if (it != voiceDatas_.end() && (*it)->sourceVoice) {
		(*it)->sourceVoice->Stop(0);  // サウンドの停止
		(*it)->sourceVoice->FlushSourceBuffers();  // バッファをクリア
	}
}

void AudioCommon::SoundPause(uint32_t voiceHandle)
{
	auto it = std::find_if(voiceDatas_.begin(), voiceDatas_.end(),
		[voiceHandle](VoiceData* data) { return data->handle == voiceHandle; });

	if (it != voiceDatas_.end() && (*it)->sourceVoice) {
		(*it)->sourceVoice->Stop(0);  // 再生を停止（ポーズ）
	}
}

void AudioCommon::SoundResume(uint32_t voiceHandle)
{
	auto it = std::find_if(voiceDatas_.begin(), voiceDatas_.end(),
		[voiceHandle](VoiceData* data) { return data->handle == voiceHandle; });

	if (it != voiceDatas_.end() && (*it)->sourceVoice) {
		(*it)->sourceVoice->Start(0);  // 再生を再開
	}
}

void AudioCommon::SetVolume(uint32_t voiceHandle, float volume)
{
	auto it = std::find_if(voiceDatas_.begin(), voiceDatas_.end(),
		[voiceHandle](VoiceData* data) { return data->handle == voiceHandle; });

	if (it != voiceDatas_.end() && (*it)->sourceVoice) {
		(*it)->sourceVoice->SetVolume(volume);  // 音量の設定
	}
}

void AudioCommon::ClearSoundData() {
	for (auto& soundData : soundDatas_) {
		if (soundData.pBuffer) {
			SoundUnload(&soundData); // バッファを解放
		}
	}
}

void AudioCommon::ClearVoiceData() {
	for (auto it = voiceDatas_.begin(); it != voiceDatas_.end(); ) {
		VoiceData* voiceData = *it;

		if (voiceData->sourceVoice) {
			voiceData->sourceVoice->Stop();         // 再生を停止
			voiceData->sourceVoice->DestroyVoice(); // ボイスリソースを解放
		}

		delete voiceData; // 動的に確保しているため解放
		it = voiceDatas_.erase(it); // イテレーターを安全に次へ進める
	}
}

void AudioCommon::ShutdownContainer() {
	ClearSoundData(); // SoundDataの解放
	ClearVoiceData(); // VoiceDataの解放
}