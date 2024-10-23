#pragma once

class DirectXCommon;
class ModelCommon
{
public:
	void Initialize(DirectXCommon* dxCommon);


public://ゲッター
	DirectXCommon* GetDirectXCommon()const { return dxCommon_; }
	
public://セッター
	

private:
	DirectXCommon* dxCommon_;
	

};

