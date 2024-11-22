#pragma once
#include <map>
#include <string>
#include <memory>
#include "Model.h"


class ModelManager
{
private://コンストラクタ等の隠蔽
	static ModelManager* instance;

	ModelManager() = default;//コンストラクタ隠蔽
	~ModelManager() = default;//デストラクタ隠蔽
	ModelManager(ModelManager&) = delete;//コピーコンストラクタ封印
	ModelManager& operator=(ModelManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	//初期化
	void Initialize();
	//終了
	void Finalize();

	//読み込み関数
	void LoadModel(const std::string& filePath, Model::ModelFormat format = Model::OBJ);
	//モデルデータ取得関数
	Model* FindModel(const std::string& filePath);

private://インスタンス

private:
	//モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models;
};

