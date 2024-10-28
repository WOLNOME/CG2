#pragma once
#include <wrl.h>
#include <xaudio2.h>
#include <cstdint>
#include <string>

#pragma comment(lib,"xaudio2.lib")
class Audio
{
private://シングルトン設定
	static Audio* instance;

	Audio() = default;
	~Audio() = default;
	Audio(Audio&) = delete;
	Audio& operator=(Audio&) = delete;
public://シングルトンインスタンス
	static Audio* GetInstance();

private://構造体

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
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 終了時
	/// </summary>
	void Finalize();

	/// <summary>
	/// 再生
	/// </summary>
	void Play();

	/// <summary>
	/// 読み込み
	/// </summary>
	/// <param name="filename"></param>
	void LoadWave(const std::string& filename);

	/// <summary>
	/// サウンドデータの解放
	/// </summary>
	void Unload();

private://メンバ変数
	//xAudio2
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_ = nullptr;
	//マスターボイス
	IXAudio2MasteringVoice* masterVoice;
	//サウンドデータ
	SoundData soundData_;
};

