#include "PointLight.h"
#include "DirectXCommon.h"

void PointLight::Update() {
	//データを転送
	data_.color = color_;
	data_.position = position_;
	data_.intensity = intencity_;
	data_.radius = radius_;
	data_.decay = decay_;
	data_.isActive = isActive_;
}
