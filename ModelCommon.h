#pragma once

class DirectXCommon;
class ModelCommon
{
public:
	void Initialize(DirectXCommon* dxCommon);


public://ゲッター
	DirectXCommon* GetCommon()const { return dxCommon_; }
private:
	DirectXCommon* dxCommon_;
};

