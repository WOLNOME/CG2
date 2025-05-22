#pragma once
#include <string>
#include "AudioCommon.h"

class Audio
{
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Audio();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="filename">Resources/はカットして</param>
	/// <param name="directoryPath">ディレクトリパス (デフォルト: "Resources/audios/")</param>
	void Initialize(const std::string& filename, const std::string& directoryPath = "Resources/audios/");

	/// <summary>
	/// 再生
	/// </summary>
	void Play(bool loop = false, float volume = 1.0f);

	/// <summary>
	/// 停止
	/// </summary>
	void Stop();

	/// <summary>
	/// 一時停止
	/// </summary>
	void Pause();

	/// <summary>
	/// 再開
	/// </summary>
	void Resume();

	/// <summary>
	/// 音量設定
	/// </summary>
	/// <param name="volume">0.0f〜1.0fの範囲で設定</param>
	void SetVolume(float volume);

private: // メンバ変数
	std::string directoryPath_;  // ディレクトリパス
	uint32_t soundDataHandle_ = 0u;  // サウンドデータハンドル
	uint32_t voiceDataHandle_ = 0u;  // ボイスデータハンドル
};
