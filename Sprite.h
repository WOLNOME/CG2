#pragma once
#include "SpriteCommon.h"
class Sprite
{
public://メンバ関数
	void Initialize(SpriteCommon* spriteCommon);
private://インスタンス
	SpriteCommon* spriteCommon_ = nullptr;
};

