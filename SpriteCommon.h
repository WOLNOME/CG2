#pragma once
#include "DirectXCommon.h"

class SpriteCommon
{
public://メンバ関数
	void Initialize(DirectXCommon* directXCommon);

public://ゲッター
	DirectXCommon* GetDirectXCommon() const { return directXCommon_; }
private://インスタンス
	DirectXCommon* directXCommon_ = nullptr;
};

