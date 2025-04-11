#include "Shape.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "TextureManager.h"
#include <numbers>
#include <cassert>

void Shape::Initialize(ShapeKind shapeKind) {
	//形状の種類
	shapeKind_ = shapeKind;
	//形状リソースの初期設定
	shapeResource_ = MakeShapeResource();
}

void Shape::Update() {
}

void Shape::Draw(uint32_t materialRootParameterIndex, uint32_t textureRootParameterIndex, int32_t textureHandle) {
	//頂点バッファビューを設定
	MainRender::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &shapeResource_.vertexBufferView);
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(materialRootParameterIndex, shapeResource_.materialResource->GetGPUVirtualAddress());
	//テクスチャがない場合は飛ばす
	shapeResource_.materialData->isTexture = (textureHandle != EOF) ? true : false;
	if (shapeResource_.materialData->isTexture) {
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(textureRootParameterIndex, TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle));
	}
	//描画
	MainRender::GetInstance()->GetCommandList()->DrawInstanced(shapeResource_.vertexNum, 1, 0, 0);
}

Shape::ShapeResource Shape::MakeShapeResource() {
	//形状リソース
	ShapeResource resource;

	switch (shapeKind_) {
	case ShapeKind::kSphere:
		resource = MakeSphereResource();
		break;
	case ShapeKind::kCube:
		resource = MakeCubeResource();
		break;
	case ShapeKind::kPlane:
		resource = MakePlaneResource();
		break;
	default:
		assert(0 && "未確認の形状が入力されています。");
		break;
	}

	return resource;
}

Shape::ShapeResource Shape::MakeSphereResource() {
	ShapeResource resource;
	//ローカル変数
	float pi = std::numbers::pi_v<float>;
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	const float kLatEvery = pi / float(kSubdivision);
	Sphere sphere;
	sphere.center = { 0.0f,0.0f,0.0f };
	sphere.radius = 1.0f;
	//頂点数を保持
	resource.vertexNum = kSubdivision * kSubdivision * 6;
	//リソースを作る
	resource.vertexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * resource.vertexNum);
	resource.materialResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
	//頂点バッファビューを作成
	resource.vertexBufferView.BufferLocation = resource.vertexResource->GetGPUVirtualAddress();
	resource.vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * resource.vertexNum);
	resource.vertexBufferView.StrideInBytes = sizeof(VertexData);
	//リソースにデータをマッピング
	resource.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&resource.vertexData));
	resource.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&resource.materialData));
	//データに書き込み
	//緯度の方向に分割
	for (UINT latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;
		//緯度の方向に分割
		for (UINT lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;
			///三角形一枚目
			//頂点にデータを入れる
			resource.vertexData[start].position.x = sphere.center.x + sphere.radius * cos(lat) * cos(lon);
			resource.vertexData[start].position.y = sphere.center.y + sphere.radius * sin(lat);
			resource.vertexData[start].position.z = sphere.center.z + sphere.radius * cos(lat) * sin(lon);
			resource.vertexData[start].position.w = 1.0f;
			resource.vertexData[start].texcoord.x = float(lonIndex) / float(kSubdivision);
			resource.vertexData[start].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			//頂点にデータを入れる
			resource.vertexData[start + 1].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon);
			resource.vertexData[start + 1].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
			resource.vertexData[start + 1].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon);
			resource.vertexData[start + 1].position.w = 1.0f;
			resource.vertexData[start + 1].texcoord.x = float(lonIndex) / float(kSubdivision);
			resource.vertexData[start + 1].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			//頂点にデータを入れる
			resource.vertexData[start + 2].position.x = sphere.center.x + sphere.radius * cos(lat) * cos(lon + kLonEvery);
			resource.vertexData[start + 2].position.y = sphere.center.y + sphere.radius * sin(lat);
			resource.vertexData[start + 2].position.z = sphere.center.z + sphere.radius * cos(lat) * sin(lon + kLonEvery);
			resource.vertexData[start + 2].position.w = 1.0f;
			resource.vertexData[start + 2].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			resource.vertexData[start + 2].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			///三角形二枚目
			//頂点にデータを入れる
			resource.vertexData[start + 3].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon);
			resource.vertexData[start + 3].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
			resource.vertexData[start + 3].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon);
			resource.vertexData[start + 3].position.w = 1.0f;
			resource.vertexData[start + 3].texcoord.x = float(lonIndex) / float(kSubdivision);
			resource.vertexData[start + 3].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			//頂点にデータを入れる
			resource.vertexData[start + 4].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon + kLonEvery);
			resource.vertexData[start + 4].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
			resource.vertexData[start + 4].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon + kLonEvery);
			resource.vertexData[start + 4].position.w = 1.0f;
			resource.vertexData[start + 4].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			resource.vertexData[start + 4].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			//頂点にデータを入れる
			resource.vertexData[start + 5].position.x = sphere.center.x + sphere.radius * cos(lat) * cos(lon + kLonEvery);
			resource.vertexData[start + 5].position.y = sphere.center.y + sphere.radius * sin(lat);
			resource.vertexData[start + 5].position.z = sphere.center.z + sphere.radius * cos(lat) * sin(lon + kLonEvery);
			resource.vertexData[start + 5].position.w = 1.0f;
			resource.vertexData[start + 5].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			resource.vertexData[start + 5].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);

			//法線情報の入力
			for (uint32_t i = 0; i < 6; i++) {
				resource.vertexData[start + i].normal.x = resource.vertexData[start + i].position.x;
				resource.vertexData[start + i].normal.y = resource.vertexData[start + i].position.y;
				resource.vertexData[start + i].normal.z = resource.vertexData[start + i].position.z;
			}

			//メモ//
			//三角形描画時には頂点を半時計周りの順番で設定する。
			//時計回りにすると表裏が逆になってしまう。
		}
	}
	resource.materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	resource.materialData->isTexture = false;
	resource.materialData->shininess = 20.0f;
	resource.materialData->uvTransform = MyMath::MakeIdentity4x4();
	return resource;
}

Shape::ShapeResource Shape::MakeCubeResource() {
	ShapeResource resource;

	return resource;
}

Shape::ShapeResource Shape::MakePlaneResource() {
	ShapeResource resource;
	//ローカル変数
	const float kHalfSize = 1.0f;
	//頂点数を保持
	resource.vertexNum = 6;
	//リソースを作る
	resource.vertexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * resource.vertexNum);
	resource.materialResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
	//頂点バッファビューを作成
	resource.vertexBufferView.BufferLocation = resource.vertexResource->GetGPUVirtualAddress();
	resource.vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * resource.vertexNum);
	resource.vertexBufferView.StrideInBytes = sizeof(VertexData);
	//リソースにデータをマッピング
	resource.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&resource.vertexData));
	resource.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&resource.materialData));
	//データに書き込み
	
	//三角形一枚目
	resource.vertexData[0].position = { -kHalfSize, 0.0f, -kHalfSize, 1.0f };
	resource.vertexData[0].texcoord = { 0.0f, 0.0f };
	resource.vertexData[1].position = { kHalfSize, 0.0f, -kHalfSize, 1.0f };
	resource.vertexData[1].texcoord = { 1.0f, 0.0f };
	resource.vertexData[2].position = { -kHalfSize, 0.0f, kHalfSize, 1.0f };
	resource.vertexData[2].texcoord = { 0.0f, 1.0f };
	//三角形二枚目
	resource.vertexData[3].position = { kHalfSize, 0.0f, -kHalfSize, 1.0f };
	resource.vertexData[3].texcoord = { 1.0f, 0.0f };
	resource.vertexData[4].position = { kHalfSize, 0.0f, kHalfSize, 1.0f };
	resource.vertexData[4].texcoord = { 1.0f, 1.0f };
	resource.vertexData[5].position = { -kHalfSize, 0.0f, kHalfSize, 1.0f };
	resource.vertexData[5].texcoord = { 0.0f, 1.0f };
	//法線は全て同じ
	for (uint32_t i = 0; i < 6; i++) {
		resource.vertexData[i].normal = { 0.0f, 1.0f, 0.0f };
	}
	//マテリアルデータ
	resource.materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	resource.materialData->isTexture = false;
	resource.materialData->shininess = 20.0f;
	resource.materialData->uvTransform = MyMath::MakeIdentity4x4();

	return resource;
}
