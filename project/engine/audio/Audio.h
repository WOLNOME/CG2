#pragma once
#include <string>
#include <functional>
#include "AudioCommon.h"
#include "Vector3.h"

class Audio {
public:
	~Audio();

	// 初期化
	void Initialize(const std::string& filename);

	/// <summary>
	/// 再生
	/// </summary>
	/// <param name="loop">ループの有無</param>
	void Play(bool loop = false);
	/// <summary>
	/// 再生中止
	/// </summary>
	void Stop();
	/// <summary>
	/// 途中で止める
	/// </summary>
	void Pause();
	/// <summary>
	/// 再開する
	/// </summary>
	void Resume();
	/// <summary>
	/// 再生中か
	/// </summary>
	/// <returns></returns>
	bool IsPlaying() const;
	/// <summary>
	/// 停止中か
	/// </summary>
	/// <returns></returns>
	bool IsPaused() const;

	// 音量制御
	void SetVolume(float volume);
	float GetVolume() const;

	// 3D サウンド
	void SetPosition(const Vector3& v);
	void SetListenerPosition(const Vector3& v);

	// コールバック
	void SetOnPlaybackEndCallback(std::function<void()> callback);

private:
	//サウンドデータ
	AudioCommon::SoundData soundData_;
	//ソースボイス
	IXAudio2SourceVoice* sourceVoice_ = nullptr;
	float volume_ = 1.0f;
	bool isPlaying_ = false;
	bool isPaused_ = false;
	std::function<void()> onPlaybackEndCallback_;
};
