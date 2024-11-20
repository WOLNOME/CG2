#include "SpotLight.h"
#include "DirectXCommon.h"

void SpotLight::Initialize()
{
	//リソースの作成
	resource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(SpotLightForPS));
	//リソースをマッピング
	resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	//データに書き込み
	data_->color = color_;
	data_->position = position_;
	data_->intensity = intencity_;
	data_->direction =  direction_;
	data_->distance = distance_;
	data_->decay = decay_;
	data_->cosAngle = cosAngle_;
	data_->cosFalloffStart = cosFalloffStart_;
}

void SpotLight::Update()
{
	//ライトの余弦をフォールオフ開始角度より大きくしないようにする処理
	if (cosFalloffStart_ < cosAngle_) {
		cosAngle_ = cosFalloffStart_ - 0.01f;
	}

	data_->color = color_;
	data_->position = position_;
	data_->intensity = intencity_;
	data_->direction = direction_.Normalized();
	data_->distance = distance_;
	data_->decay = decay_;
	data_->cosAngle = cosAngle_;
	data_->cosFalloffStart = cosFalloffStart_;
}

