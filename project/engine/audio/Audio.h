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

	// 再生制御
	void Play(bool loop = false);
	void Stop();
	void Pause();
	void Resume();
	bool IsPlaying() const;
	bool IsPaused() const;

	// 音量制御
	void SetVolume(float volume);
	float GetVolume() const;

	/// <summary>
	/// だんだん大きくなる
	/// </summary>
	/// <param name="duration">全体時間</param>
	/// <param name="min">最小音量</param>
	/// <param name="max">最大音量</param>
	void FadeIn(float duration, float min = 0.0f, float max = 1.0f);
	/// <summary>
	/// だんだん小さくなる
	/// </summary>
	/// <param name="duration">全体時間</param>
	/// <param name="min">最小音量</param>
	/// <param name="max">最大音量</param>
	void FadeOut(float duration, float min = 0.0f, float max = 1.0f);

	// 3D サウンド
	void SetPosition(const Vector3& v);
	void SetListenerPosition(const Vector3& v);

	// コールバック
	void SetOnPlaybackEndCallback(std::function<void()> callback);

private:
	AudioCommon::SoundData soundData_;
	float volume_ = 1.0f;
	bool isPlaying_ = false;
	bool isPaused_ = false;
	std::function<void()> onPlaybackEndCallback_;
};
