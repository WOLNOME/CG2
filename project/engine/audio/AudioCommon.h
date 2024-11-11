#pragma once
#include <wrl.h>
#include <xaudio2.h>
#include <cstdint>
#include <string>
#include <unordered_map>

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
		// XAudio2 のソースボイス
		IXAudio2SourceVoice* sourceVoice = nullptr;
	};

public:
	//初期化
	void Initialize();
	//終了
	void Finalize();
	//音声データの読み込み
	SoundData SoundLoadWave(const std::string& filename);
	//サウンドの解放
	void SoundUnload(SoundData* soundData);
	// サウンドの再生
	void SoundPlayWave(SoundData& soundData, bool loop = false);
	// サウンドの一時停止
	void SoundPause(SoundData& soundData);
	// サウンドの再開
	void SoundResume(SoundData& soundData);
	// サウンドの停止
	void SoundStop(SoundData& soundData);
	// 音量の設定
	void SetVolume(SoundData& soundData, float volume);
	// ループ再生の設定
	void SetLoop(SoundData& soundData, bool loop);

private://非公開メンバ関数
	//全サウンドデータの解放
	void ClearSoundDatas();

private://メンバ変数
	//xAudio2
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ = nullptr;
	//マスターボイス
	IXAudio2MasteringVoice* masterVoice;
	//全サウンドデータ
	std::unordered_map<std::string, SoundData> soundDatas_;
};

