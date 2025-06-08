#include "DirectionalLight.h"
#include "ImGuiManager.h"
#include "StringUtility.h"

void DirectionalLight::Initialize() {
}

void DirectionalLight::Update() {
	//データを転送
	data_.color = color_;
	data_.direction = direction_;
	data_.intensity = intencity_;
	data_.isActive = isActive_;
}

void DirectionalLight::DebugWithImGui(const std::wstring& _name) {
#ifdef _DEBUG
	ImGui::Begin("平行光源");
	if (ImGui::CollapsingHeader(StringUtility::ConvertString(_name).c_str())) {
		ImGui::ColorEdit4("色", &color_.x);
		ImGui::DragFloat3("向き", &direction_.x, 0.01f);
		ImGui::DragFloat("輝度", &intencity_, 0.01f, 0.0f, 100.0f);
		ImGui::Checkbox("適用するか", &isActive_);
	}
	ImGui::End();
#endif // _DEBUG

}
