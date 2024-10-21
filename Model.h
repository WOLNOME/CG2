#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "Function.h"

class ModelCommon;
class Object3d;
class Model
{
public:
	void Initialize(ModelCommon* modelCommon, Object3d* object3d);


private:
	ModelCommon* modelCommon_;
	Object3d* object3d_;
	//モデル用リソース
	Object3d::Struct::ModelResource modelResource_;


};

