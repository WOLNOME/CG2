#pragma once
#include <wrl.h>
#include <xaudio2.h>
#include <cstdint>
#include <string>

#pragma comment(lib,"xaudio2.lib")
class AudioCommon
{
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

public:
	~AudioCommon();

	void Initialize();

	//音声データの読み込み
	static SoundData SoundLoadWave(const std::string& filename);
	//サウンドの再生
	void SoundPlayWave(const SoundData& soundData);

	void SoundUnload(SoundData* soundData);

private://メンバ変数
	//xAudio2
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ = nullptr;
	//マスターボイス
	IXAudio2MasteringVoice* masterVoice;
	//全サウンドデータ

};

