#pragma once

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
	void Initialize();
	//終了
	void Finalize();

public://ゲッター

public://セッター
	

private:

};

