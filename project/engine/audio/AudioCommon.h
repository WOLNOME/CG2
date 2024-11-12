#pragma once
#include <wrl.h>
#include <xaudio2.h>
#include <cstdint>
#include <string>
#include <array>
#include <set>

#pragma comment(lib,"xaudio2.lib")
class AudioCommon
{
private://シングルトン
	static AudioCommon* instance;

	AudioCommon() = default;//コンストラクタ隠蔽
	~AudioCommon() = default;//デストラクタ隠蔽
	AudioCommon(AudioCommon&) = delete;//コピーコンストラクタ封印
	AudioCommon& operator=(AudioCommon&) = delete;//コピー代入演算子封印
public://シングルトン
	static AudioCommon* GetInstance();
private://非公開構造体
	//チャンクヘッダ
	struct ChunkHeader
	{
		char id[4];
		int32_t size;
	};

	//RIFFヘッダチャンク
	struct RiffHeader
	{
		ChunkHeader chunk;
		char type[4];
	};

	//FMTチャンク
	struct FormatChunk
	{
		ChunkHeader chunk;
		WAVEFORMATEX fmt;
	};
public://公開構造体
	//音声データ
	struct SoundData
	{
		//波形フォーマット
		WAVEFORMATEX wfex;
		//バッファの先頭アドレス
		BYTE* pBuffer;
		//バッファのサイズ
		unsigned int bufferSize;
		//ファイルパス
		std::string name;
	};
	// 再生データ
	struct VoiceData {
		//アクセスハンドル
		uint32_t handle = 0u;
		//ソースボイス
		IXAudio2SourceVoice* sourceVoice = nullptr;
	};

public:
	// サウンドデータの最大数
	static const int kMaxSoundData = 128;

	//初期化
	void Initialize();
	//終了
	void Finalize();
	//音声データの読み込み
	uint32_t SoundLoadWave(const std::string& filename);
	//サウンドの再生
	uint32_t SoundPlayWave(uint32_t soundDataHandle, bool loop = false, float volume = 1.0f);
	//サウンドの停止
	void SoundStop(uint32_t voiceHandle);
	//サウンドの一時停止
	void SoundPause(uint32_t voiceHandle);
	//サウンドの再生再開
	void SoundResume(uint32_t voiceHandle);
	//音量の調整
	void SetVolume(uint32_t voiceHandle, float volume);



private:
	void SoundUnload(SoundData* soundData);
	void ClearSoundData();
	void ClearVoiceData();
	void ShutdownContainer();

private://メンバ変数
	//xAudio2
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ = nullptr;
	//マスターボイス
	IXAudio2MasteringVoice* masterVoice;
	//サウンドデータコンテナ
	std::array<SoundData, kMaxSoundData> soundDatas_;
	//ボイスデータコンテナ
	std::set<VoiceData*> voiceDatas_;
	//サウンドデータコンテナの開始位置
	const int kStartSoundDataIndex = 1;

};

