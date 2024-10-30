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
	/// <param name="audioCommon"></param>
	/// <param name="filename">Resources/はカットして</param>
	void Initialize(const std::string& filename);
	
	/// <summary>
	/// 再生
	/// </summary>
	void Play();

private://インスタンス

private://メンバ変数
	AudioCommon::SoundData soundData_;
};

