#include "WorldTransform.h"
#include "DirectXCommon.h"
#include "MyMath.h"

void WorldTransform::Initialize()
{
	//リソースの作成
	resource_ = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(WorldTransformationMatrixForVS));
	//リソースをマッピング
	resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	//データに書き込み
	data_->matWorld = MyMath::MakeIdentity4x4();
	data_->matWorldInverseTranspose = MyMath::MakeIdentity4x4();
}

void WorldTransform::UpdateMatrix(const Matrix4x4& local)
{
	// スケール、回転、平行移動を合成して行列を計算する
	matWorld_ = local * MyMath::MakeAffineMatrix(scale_, rotate_, translate_);

	// 親があれば親のワールド行列を掛ける
	if (parent_) {
		matWorld_ = MyMath::Multiply(matWorld_, parent_->matWorld_);
	}

	// 定数バッファに転送する
	data_->matWorld = matWorld_;
	data_->matWorldInverseTranspose = MyMath::Transpose(MyMath::Inverse(matWorld_));
}
