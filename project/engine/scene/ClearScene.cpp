#include "ClearScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "LineDrawerCommon.h"
#include "SceneManager.h"

void ClearScene::Initialize()
{
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

}

void ClearScene::Finalize()
{
}

void ClearScene::Update()
{
	if (input_->TriggerKey(DIK_TAB)) {
		sceneManager_->SetNextScene("TITLE");
	}


#ifdef _DEBUG
	ImGui::Begin("scene");
	ImGui::Text("%s", "CLEAR");
	ImGui::End();
#endif // _DEBUG
}

void ClearScene::Draw()
{
}
