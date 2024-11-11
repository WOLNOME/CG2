#pragma once
#include <wrl.h>
#include <xaudio2.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <set>
#include <memory>


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

	};
	//ボイスデータ
	struct VoiceData
	{
		//ハンドル
		uint32_t handle = 0u;
		//ソースボイス
		std::unique_ptr<IXAudio2SourceVoice> sourceVoice;
	};


public:
	//初期化
	void Initialize();
	//終了
	void Finalize();
	//音声データの読み込み
	uint32_t SoundLoadWave(const std::string& filename);
	// サウンドの再生
	uint32_t SoundPlayWave(uint32_t soundDataHandle, bool loop = false);
	// サウンドの一時停止
	void SoundPause(uint32_t voiceDataHandle);
	// サウンドの再開
	void SoundResume(uint32_t voiceDataHandle);
	// サウンドの停止
	void SoundStop(uint32_t voiceDataHandle);
	// 音量の設定
	void SetVolume(uint32_t voiceDataHandle, float volume);
	// ループ再生の設定
	void SetLoop(uint32_t soundDataHandle, uint32_t voiceDataHandle, bool loop);

private://非公開メンバ関数

private://メンバ変数
	//xAudio2
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ = nullptr;
	//マスターボイス
	IXAudio2MasteringVoice* masterVoice;
	//全サウンドデータ
	std::unordered_map<std::string, SoundData> soundDatas_;
	//全ボイスデータ
	std::set<VoiceData*> voiceDatas_;

};

