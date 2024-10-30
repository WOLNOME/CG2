#include "ModelCommon.h"
#include "DirectXCommon.h"
#include "Camera.h"

ModelCommon* ModelCommon::instance = nullptr;

ModelCommon* ModelCommon::GetInstance()
{
	if (instance == nullptr) {
		instance = new ModelCommon;
	}
	return instance;
}

void ModelCommon::Initialize(DirectXCommon* dxCommon)
{
	//引数で受け取ってメンバ変数に記録する
	dxCommon_ = dxCommon;

}

void ModelCommon::Finalize()
{
	delete instance;
	instance = nullptr;
}
