#pragma once
#include "BaseScene.h"
#include "DevelopCamera.h"
#include "json.hpp"
#include "WorldTransform.h"
#include <vector>
#include <string>

using json = nlohmann::json;

class ParticleCreatorScene : public BaseScene {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// 終了時
	/// </summary>
	void Finalize() override;
	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;
	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;
	/// <summary>
	/// テキスト描画
	/// </summary>
	void TextDraw() override;

private://ImGui操作
	//最初の画面
	void StartWithImGui();
	//新規作成
	void GenerateWithImGui();
	//編集
	void EditWithImGui();
	//リセット
	void ResetWithImGui();
	//セーブ
	void SaveWithImGui();
	//エディター本体
	void Editor();

private://必要なオブジェクト
	//インプット
	Input* input_ = nullptr;
	//開発用カメラ
	std::unique_ptr<DevelopCamera> camera_ = nullptr;

	//天球と地面
	WorldTransform wtSkydome_;
	std::unique_ptr<Object3d> skydome_ = nullptr;
	WorldTransform wtGround_;
	uint32_t textureHandleGround_ = EOF;
	std::unique_ptr<Object3d> ground_ = nullptr;

	//エミッターの線描画
	std::unique_ptr<LineDrawer> lineEmitter_ = nullptr;

private:
	// 編集するパーティクル
	std::unique_ptr<Particle> particle_ = nullptr;
	// 編集するパラメーター
	json editParam_;
	

private://ImGui操作で必要な変数
	std::vector<std::string> textureFiles_;
	std::vector<std::string> particleFiles_;
	std::string jsonFileName_;
	bool showFileDialog_ = false;
	bool isGenerateMode_ = false;
	bool isEditMode_ = false;
	bool isReset_ = false;
	bool isSave_ = false;
	bool checkContinue_ = false;
	bool checkSameName_ = false;
	bool checkEditName_ = false;
	bool displayLineEmitter_ = true;
};

