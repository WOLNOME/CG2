#include "DevelopCamera.h"
#include "ImGuiManager.h"
#include "Input.h"

void DevelopCamera::Initialize()
{
	//基盤の初期化
	BaseCamera::Initialize();
	//インプット
	input_ = Input::GetInstance();

}

void DevelopCamera::Update()
{
	////開発用カメラのマウス操作処理
	//スクロールで前進後退
	transform.translate += GetForwardDirection() * (input_->GetMouseScrollCount() * 1.0f);
	//ドラッグアンドドロップでカメラの向きを変える
	if (input_->PushMouseButton(MouseButton::RightButton)) {
		if (input_->TriggerMouseButton(MouseButton::RightButton)) {
			start = input_->GetMousePosition();
		}
		transform.rotate.x += (input_->GetMousePosition().y - start.y) * 0.005f;
		transform.rotate.y += -(input_->GetMousePosition().x - start.x) * 0.005f;
	}

	//行列の更新
	BaseCamera::UpdateMatrix();

#ifdef _DEBUG

	ImGui::Begin("DevelopCamera");
	ImGui::DragFloat3("Translate", &transform.translate.x, 0.01f);
	ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.01f);
	ImGui::End();

	ImGui::Begin("mouse");
	if (input_->PushMouseButton(MouseButton::LeftButton)) {
		ImGui::Text("LEFT");
	}
	if (input_->PushMouseButton(MouseButton::MiddleButton)) {
		ImGui::Text("MIDDLE");
	}
	if (input_->PushMouseButton(MouseButton::RightButton)) {
		ImGui::Text("RIGHT");
	}
	ImGui::Text("ScrollValue : %f", input_->GetMouseScrollCount());
	ImGui::Text("Delta : { %f,%f }", input_->GetMouseDelta().x, input_->GetMouseDelta().y);
	ImGui::End();

#endif // _DEBUG


}
