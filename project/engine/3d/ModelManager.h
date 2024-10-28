#pragma once
#include <map>
#include <string>
#include <memory>

class Model;
class DirectXCommon;
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
	void Initialize(DirectXCommon* dxCommon);
	//終了
	void Finalize();

	//読み込み関数
	void LoadModel(const std::string& filePath);
	//モデルデータ取得関数
	Model* FindModel(const std::string& filePath);

private:
	//モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models;
	

};

