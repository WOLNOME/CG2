#include "GamePlayCamera.h"
#include "ImGuiManager.h"

void GamePlayCamera::Initialize()
{
	//基盤の初期化
	BaseCamera::Initialize();



}

void GamePlayCamera::Update()
{
	//行列の更新
	BaseCamera::UpdateMatrix();

#ifdef _DEBUG
	ImGui::Begin("gpCamera");
	ImGui::DragFloat3("translate", &transform.translate.x, 0.01f);
	ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f);
	ImGui::End();
#endif // _DEBUG

}
