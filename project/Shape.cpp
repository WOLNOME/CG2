#include "Shape.h"
#include <numbers>
#include "DirectXCommon.h"
#include "MainRender.h"
#include "Object3dCommon.h"
#include "BaseCamera.h"
#include "MyMath.h"


Shape::Shape()
{
}

Shape::~Shape()
{
}

void Shape::Initialize(ShapeKind kind)
{
	kind_ = kind;

	//リソース作成
	shapeResource_ = MakeShapeResource();
}

void Shape::Draw(const WorldTransform& worldTransform, const BaseCamera& camera, const SceneLight* sceneLight)
{
	//ノンアニメーション設定
	Object3dCommon::GetInstance()->SettingCommonDrawing(Object3dCommon::kNone);

	//頂点バッファビューを設定
	MainRender::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &shapeResource_.vertexBufferView);
	//インデックスバッファビューを設定
	MainRender::GetInstance()->GetCommandList()->IASetIndexBuffer(&shapeResource_.indexBufferView);

	//SceneLightCBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(5, sceneLight->GetSceneLightConstBuffer()->GetGPUVirtualAddress());
	//WorldTransformCBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());
	//CameraからビュープロジェクションCBufferの場所設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(2, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());
	//Cameraからカメラ座標CBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(4, camera.GetCameraPositionConstBuffer()->GetGPUVirtualAddress());
	//マテリアルの設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, shapeResource_.materialResource->GetGPUVirtualAddress());

	//描画
	MainRender::GetInstance()->GetCommandList()->DrawIndexedInstanced(UINT(shapeResource_.indexData.size()), 1, 0, 0, 0);

}

Shape::ShapeResource Shape::MakeShapeResource()
{
	ShapeResource resource;
	switch (kind_)
	{
	case Shape::kSphere:
		//分割数
		const int kSubdivision = 15;
		//インデックスのサイズを決める
		resource.indexData.resize(kSubdivision * kSubdivision * 6);

		//リソースを作る
		resource.vertexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * (kSubdivision * kSubdivision * 4));
		resource.indexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * (kSubdivision * kSubdivision * 6));
		resource.materialResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
		//頂点バッファビューを作成
		resource.vertexBufferView.BufferLocation = resource.vertexResource->GetGPUVirtualAddress();
		resource.vertexBufferView.SizeInBytes = sizeof(VertexData) * (kSubdivision * kSubdivision * 4);
		resource.vertexBufferView.StrideInBytes = sizeof(VertexData);
		//インデックスバッファビューを作成
		resource.indexBufferView.BufferLocation = resource.vertexResource->GetGPUVirtualAddress();
		resource.indexBufferView.SizeInBytes = sizeof(uint32_t) * (kSubdivision * kSubdivision * 6);
		resource.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		//リソースにデータを書き込む
		resource.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&resource.vertexData));
		resource.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&resource.indexData));
		resource.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&resource.materialData));
		///頂点の座標を決めていく
		//経度分割1つ分の角度
		const float kLonEvery = std::numbers::pi_v<float> *2.0f / float(kSubdivision);
		//緯度分割1つ分の角度
		const float kLatEvery = std::numbers::pi_v<float> / float(kSubdivision);
		//球の情報
		Sphere sphere;
		sphere.center = { 0.0f,0.0f,0.0f };
		sphere.radius = 1.0f;
		//緯度の方向に分割
		for (UINT latIndex = 0; latIndex < kSubdivision; ++latIndex) {
			float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
			//経度の方向に分割
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
				//頂点にデータを入れる
				resource.vertexData[start + 3].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon + kLonEvery);
				resource.vertexData[start + 3].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
				resource.vertexData[start + 3].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon + kLonEvery);
				resource.vertexData[start + 3].position.w = 1.0f;
				resource.vertexData[start + 3].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
				resource.vertexData[start + 3].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);

				//法線情報の入力(球の中心→頂点のベクトル)
				for (uint32_t i = 0; i < 4; i++) {
					resource.vertexData[start + i].normal.x = resource.vertexData[start + i].position.x;
					resource.vertexData[start + i].normal.y = resource.vertexData[start + i].position.y;
					resource.vertexData[start + i].normal.z = resource.vertexData[start + i].position.z;
				}

				//インデックスデータの割り当て
				resource.indexData[start] = start;
				resource.indexData[start + 1] = start + 1;
				resource.indexData[start + 2] = start + 2;
				resource.indexData[start + 3] = start + 1;
				resource.indexData[start + 4] = start + 3;
				resource.indexData[start + 5] = start + 2;

				//メモ//
				//三角形描画時には頂点を半時計周りの順番で設定する。
				//時計回りにすると表裏が逆になってしまう。
			}
		}
		//マテリアルデータに入力
		resource.materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		resource.materialData->isTexture = false;
		resource.materialData->uvTransform = MyMath::MakeIdentity4x4();
		resource.materialData->shininess = 20.0f;

		break;
	case Shape::kCube:

		break;
	default:
		break;
	}


	return resource;
}
