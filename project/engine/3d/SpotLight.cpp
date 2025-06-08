#include "SpotLight.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "StringUtility.h"

void SpotLight::Update() {
	//ライトの余弦をフォールオフ開始角度より大きくしないようにする処理
	if (cosFalloffStart_ < cosAngle_) {
		cosAngle_ = cosFalloffStart_ - 0.01f;
	}

	data_.color = color_;
	data_.position = position_;
	data_.intensity = intencity_;
	data_.direction = direction_.Normalized();
	data_.distance = distance_;
	data_.decay = decay_;
	data_.cosAngle = cosAngle_;
	data_.cosFalloffStart = cosFalloffStart_;
	data_.isActive = isActive_;
}

void SpotLight::DebugWithImGui(const std::wstring& _name) {
	ImGui::Begin("指向性光源(スポットライト)");
	if (ImGui::CollapsingHeader(StringUtility::ConvertString(_name).c_str())) {
		ImGui::ColorEdit4("色", &color_.x);
		ImGui::DragFloat3("座標", &position_.x, 0.01f);
		ImGui::DragFloat("輝度", &intencity_, 0.01f, 0.0f, 100.0f);
		ImGui::DragFloat3("向き", &direction_.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat("距離", &distance_, 0.01f, 0.0f, 100.0f);
		ImGui::DragFloat("減衰率", &decay_, 0.01f, 0.0f, 100.0f);
		ImGui::DragFloat("余弦(扇の大きさ)", &cosAngle_, 0.01f, -1.0f, cosFalloffStart_ - 0.01f);
		ImGui::DragFloat("フォールオフの開始角度", &cosFalloffStart_, 0.01f, 0.0f, 2.0f);
		ImGui::Checkbox("適用するか", &isActive_);
	}
	ImGui::End();

}

