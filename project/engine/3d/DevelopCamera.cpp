#include "DevelopCamera.h"
#include "ImGuiManager.h"

void DevelopCamera::Initialize()
{
	//基盤の初期化
	BaseCamera::Initialize();

}

void DevelopCamera::Update()
{
	//行列の更新
	BaseCamera::UpdateMatrix();

#ifdef _DEBUG

	ImGui::Begin("DevelopCamera");
	ImGui::DragFloat3("Translate", &transform.translate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.01f);
	ImGui::End();


#endif // _DEBUG


}
