#pragma once
#include "BaseScene.h"
#include "AbstractSceneFactory.h"
#include "SceneTransitionAnimation.h"
#include "Audio.h"
#include <string>
#include <memory>
//シーン管理
class SceneManager {
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
	/// 遷移幕描画
	/// </summary>
	void CurtainDraw();
	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

private:
	//シーン切り替え
	void ChangeScene();
public:
	//次シーンのセット
	void SetNextScene(const std::string& nextSceneName, SceneTransitionAnimation::TransitionType transitionType = SceneTransitionAnimation::TransitionType::FADE, uint32_t frame = 60);

private:
	//今のシーン
	BaseScene* scene_ = nullptr;
	//次のシーン
	BaseScene* nextScene_ = nullptr;
	//シーンファクトリー
	AbstractSceneFactory* sceneFactory_ = nullptr;
	//シーン遷移アニメーション
	std::unique_ptr<SceneTransitionAnimation> sceneTransitionAnimation_ = nullptr;

};

