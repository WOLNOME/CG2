#include "Model.h"
#include <fstream>
#include <sstream>
#include "ModelCommon.h"
#include "Object3d.h"

void Model::Initialize(ModelCommon* modelCommon, Object3d* object3d)
{
	//引数で受け取ってメンバ変数に記録する
	modelCommon_ = modelCommon;
	object3d_ = object3d;

	//モデルリソースの初期設定
	modelResource_ = object3d_->MakeModelResource("Resources", "plane.obj");

}