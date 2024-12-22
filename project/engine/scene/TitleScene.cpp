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

}

void TitleScene::Finalize()
{

}

void TitleScene::Update()
{
	if (input_->TriggerKey(DIK_TAB)) {
		sceneManager_->SetNextScene("GAMEPLAY");
	}
	

#ifdef _DEBUG
	ImGui::Begin("scene");
	ImGui::Text("%s", "TITLE");
	ImGui::End();
#endif // _DEBUG
}

void TitleScene::Draw()
{
	
}
