#include "DirectionalLight.h"
#include "DirectXCommon.h"
#include "MyMath.h"

void DirectionalLight::Initialize()
{
	//リソースの作成
	resource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(DirectionalLightForPS));
	//リソースをマッピング
	resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	//データに書き込み
	data_->color = color_;
	data_->direction = direction_;
	data_->intensity = intencity_;
}

void DirectionalLight::Update()
{
	//データを転送
	data_->color = color_;
	data_->direction = direction_;
	data_->intensity = intencity_;
}
