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

void ModelCommon::Initialize()
{
}

void ModelCommon::Finalize()
{
	delete instance;
	instance = nullptr;
}
