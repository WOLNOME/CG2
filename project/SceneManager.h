#pragma once
#include "BaseScene.h"
//現存のシーン
#include "TitleScene.h"
#include "GamePlayScene.h"

//シーン管理
class SceneManager
{
private://コンストラクタ等の隠蔽
	static SceneManager* instance;

	SceneManager() = default;//コンストラクタ隠蔽
	~SceneManager() = default;//デストラクタ隠蔽
	SceneManager(SceneManager&) = delete;//コピーコンストラクタ封印
	SceneManager& operator=(SceneManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static SceneManager* GetInstance();
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

private:
	//シーン切り替え
	void ChangeScene();
public:
	//次シーン予約
	void SetNextScene(BaseScene* nextScene) { nextScene_ = nextScene; }

private:
	//今のシーン
	BaseScene* scene_ = nullptr;
	//次のシーン
	BaseScene* nextScene_ = nullptr;

};

