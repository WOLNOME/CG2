#pragma once

class DirectXCommon;
class ModelCommon
{
private://シングルトン
	static ModelCommon* instance;

	ModelCommon() = default;//コンストラクタ隠蔽
	~ModelCommon() = default;//デストラクタ隠蔽
	ModelCommon(ModelCommon&) = delete;//コピーコンストラクタ封印
	ModelCommon& operator=(ModelCommon&) = delete;//コピー代入演算子封印
public://シングルトン
	static ModelCommon* GetInstance();
public:
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//終了
	void Finalize();

public://ゲッター
	DirectXCommon* GetDirectXCommon()const { return dxCommon_; }
	
public://セッター
	

private:
	DirectXCommon* dxCommon_;
	

};

