#include "TitleScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "LineDrawerCommon.h"
#include "SceneManager.h"

void TitleScene::Initialize()
{
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//生成と初期化
	textureHandleTitle_ = TextureManager::GetInstance()->LoadTexture("title.png");
	spriteTitle_ = std::make_unique<Sprite>();
	spriteTitle_->Initialize(textureHandleTitle_);
}

void TitleScene::Finalize()
{
}

void TitleScene::Update()
{
	if (input_->TriggerKey(DIK_TAB)) {
		sceneManager_->SetNextScene("GAMEPLAY");
	}

	spriteTitle_->Update();

#ifdef _DEBUG
	ImGui::Begin("scene");
	ImGui::Text("%s", "TITLE");
	ImGui::End();
#endif // _DEBUG
}

void TitleScene::Draw()
{
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
	///------------------------------///

	//線描画共通描画設定
	LineDrawerCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓線描画開始↓↓↓↓
	///------------------------------///

	

	///------------------------------///
	///↑↑↑↑線描画終了↑↑↑↑
	///------------------------------///

	//スプライトの共通描画設定
	SpriteCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓スプライト描画開始↓↓↓↓
	///------------------------------///

	spriteTitle_->Draw();

	///------------------------------///
	///↑↑↑↑スプライト描画終了↑↑↑↑
	///------------------------------///
}
