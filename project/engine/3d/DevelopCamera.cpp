#include "DevelopCamera.h"
#include "ImGuiManager.h"
#include "Input.h"
#include <algorithm>
#include <numbers>

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
	transform.translate += GetForwardDirection() * (input_->GetMouseScrollCount() * 1.3f);
	//ドラッグアンドドロップでカメラの向きを変える
	if (input_->PushMouseButton(MouseButton::RightButton)) {
		//マウスの移動幅
		Vector2 moveValue = input_->GetMousePosition();
		//デッドゾーン
		float deadZone = 2.0f;
		if (moveValue.Length() > deadZone) {
			transform.rotate.x += moveValue.y * 0.001f;
			transform.rotate.y += moveValue.x * 0.001f;
		}

		//WASDでカメラ移動
		if (input_->PushKey(DIK_W)) {
			transform.translate += GetForwardDirection() * 0.1f;
		}
		if (input_->PushKey(DIK_A)) {
			transform.translate += GetLeftDirection() * 0.1f;
		}
		if (input_->PushKey(DIK_S)) {
			transform.translate += GetBackDirection() * 0.1f;
		}
		if (input_->PushKey(DIK_D)) {
			transform.translate += GetRightDirection() * 0.1f;
		}


	}
	//カメラの回転制限
	const float maxPitch = (std::numbers::pi_v<float> / 2.0f) - 0.01f;
	transform.rotate.x = std::clamp(transform.rotate.x, -maxPitch, maxPitch);


	//行列の更新
	BaseCamera::UpdateMatrix();

}

void DevelopCamera::DebugWithImGui() {
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
