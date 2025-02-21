#include "Skybox.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "TextureManager.h"

void Skybox::Initialize() {
	//スカイボックスリソースの初期設定
	skyboxResource_ = MakeSkyboxResource();
}

void Skybox::Update() {
}

void Skybox::Draw(WorldTransform& worldTransform, const  BaseCamera& camera, int32_t textureHandle) {
	// 頂点バッファビューを設定
	MainRender::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &skyboxResource_.vertexBufferView);
	// インデックスバッファビューを設定
	MainRender::GetInstance()->GetCommandList()->IASetIndexBuffer(&skyboxResource_.indexBufferView);
	// WorldTransformCBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, worldTransform.GetConstBuffer()->GetGPUVirtualAddress());
	// カメラCBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, camera.GetViewProjectionConstBuffer()->GetGPUVirtualAddress());
	// テクスチャを設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle));
	// 描画
	MainRender::GetInstance()->GetCommandList()->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

Skybox::SkyboxResource Skybox::MakeSkyboxResource() {
	//形状リソース
	SkyboxResource resource;

	//頂点数を保持
	resource.vertexNum = 24;
	//リソースを作る
	resource.vertexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * resource.vertexNum);
	resource.indexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * 36);
	//頂点バッファビューを作成
	resource.vertexBufferView.BufferLocation = resource.vertexResource->GetGPUVirtualAddress();
	resource.vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * resource.vertexNum);
	resource.vertexBufferView.StrideInBytes = sizeof(VertexData);
	//インデックスバッファビューを作成
	resource.indexBufferView.BufferLocation = resource.indexResource->GetGPUVirtualAddress();
	resource.indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * 36);
	resource.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	//リソースにデータをマッピング
	resource.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&resource.vertexData));
	resource.indexResource->Map(0, nullptr, reinterpret_cast<void**>(&resource.indexData));
	//右面。描画インデックスは[0,1,2][2,1,3]で内側を向く
	resource.vertexData[0].position = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	resource.vertexData[1].position = Vector4(1.0f, 1.0f, -1.0f, 1.0f);
	resource.vertexData[2].position = Vector4(1.0f, -1.0f, 1.0f, 1.0f);
	resource.vertexData[3].position = Vector4(1.0f, -1.0f, -1.0f, 1.0f);
	//左面。描画インデックスは[4,5,6][6,5,7]で内側を向く
	resource.vertexData[4].position = Vector4(-1.0f, 1.0f, -1.0f, 1.0f);
	resource.vertexData[5].position = Vector4(-1.0f, 1.0f, 1.0f, 1.0f);
	resource.vertexData[6].position = Vector4(-1.0f, -1.0f, -1.0f, 1.0f);
	resource.vertexData[7].position = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);
	//前面。描画インデックスは[8,9,10][10,9,11]で内側を向く
	resource.vertexData[8].position = Vector4(-1.0f, 1.0f, 1.0f, 1.0f);
	resource.vertexData[9].position = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	resource.vertexData[10].position = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);
	resource.vertexData[11].position = Vector4(1.0f, -1.0f, 1.0f, 1.0f);
	//後面。描画インデックスは[12,13,14][14,13,15]で内側を向く
	resource.vertexData[12].position = Vector4(1.0f, 1.0f, -1.0f, 1.0f);
	resource.vertexData[13].position = Vector4(-1.0f, 1.0f, -1.0f, 1.0f);
	resource.vertexData[14].position = Vector4(1.0f, -1.0f, -1.0f, 1.0f);
	resource.vertexData[15].position = Vector4(-1.0f, -1.0f, -1.0f, 1.0f);
	// 上面。
	resource.vertexData[16].position = Vector4(-1.0f, 1.0f, -1.0f, 1.0f);
	resource.vertexData[17].position = Vector4(1.0f, 1.0f, -1.0f, 1.0f); 
	resource.vertexData[18].position = Vector4(-1.0f, 1.0f, 1.0f, 1.0f); 
	resource.vertexData[19].position = Vector4(1.0f, 1.0f, 1.0f, 1.0f);  

	// 下面。
	resource.vertexData[20].position = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);
	resource.vertexData[21].position = Vector4(1.0f, -1.0f, 1.0f, 1.0f); 
	resource.vertexData[22].position = Vector4(-1.0f, -1.0f, -1.0f, 1.0f);
	resource.vertexData[23].position = Vector4(1.0f, -1.0f, -1.0f, 1.0f);

	 // インデックスデータを設定
	uint32_t indices[] = {
		0, 1, 2, 2, 1, 3, // 右面
		4, 5, 6, 6, 5, 7, // 左面
		8, 9, 10, 10, 9, 11, // 前面
		12, 13, 14, 14, 13, 15, // 後面
		16, 17, 18, 18, 17, 19, // 上面
		20, 21, 22, 22, 21, 23 // 下面
	};
	memcpy(resource.indexData, indices, sizeof(indices));


	//値を返却
	return resource;
}
