#include "PointLight.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "StringUtility.h"

void PointLight::Update() {
	//データを転送
	data_.color = color_;
	data_.position = position_;
	data_.intensity = intencity_;
	data_.radius = radius_;
	data_.decay = decay_;
	data_.isActive = isActive_;
}

void PointLight::DebugWithImGui(const std::wstring& _name) {
	ImGui::Begin("点光源");
	if (ImGui::CollapsingHeader(StringUtility::ConvertString(_name).c_str())) {
		ImGui::ColorEdit4("色", &color_.x);
		ImGui::DragFloat3("座標", &position_.x, 0.01f);
		ImGui::DragFloat("輝度", &intencity_, 0.01f, 0.0f, 100.0f);
		ImGui::DragFloat("半径", &radius_, 0.01f, 0.0f, 20.0f);
		ImGui::DragFloat("減衰率", &decay_, 0.01f, 0.0f,100.0f);
		ImGui::Checkbox("適用するか", &isActive_);
	}
	ImGui::End();

}
