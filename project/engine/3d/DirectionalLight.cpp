#include "DirectionalLight.h"

void DirectionalLight::Initialize() {
}

void DirectionalLight::Update() {
	//データを転送
	data_.color = color_;
	data_.direction = direction_;
	data_.intensity = intencity_;
	data_.isActive = isActive_;
}
