#include "PointLight.h"
#include "DirectXCommon.h"
#include "MyMath.h"

void PointLight::Initialize()
{
	//リソースの作成
	resource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(PointLightForPS));
	//リソースをマッピング
	resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	//データに書き込み
	data_->color = color_;
	data_->position = position_;
	data_->intensity = intencity_;
}

void PointLight::Update()
{
	//データを転送
	data_->color = color_;
	data_->position = position_;
	data_->intensity = intencity_;
}
