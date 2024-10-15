#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	//スプライト共通部のインスタンス取得
	spriteCommon_ = spriteCommon;

	//リソースを作る
	vertexResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::VertexData) * 6);
	indexResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
	materialResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::Material));
	transformationMatrixResource = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::TransformationMatrix));

	//バッファービューを作る
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(Struct::VertexData) * 6;
	vertexBufferView.StrideInBytes = sizeof(Struct::VertexData);
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	//リソースにデータをセット
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	///データに書き込む
	//頂点データ
	vertexData[0].position = { 0.0f,1.0f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[2].position = { 1.0f,1.0f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[3].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[3].texcoord = { 0.0f,0.0f };
	vertexData[4].position = { 1.0f,0.0f,0.0f,1.0f };
	vertexData[4].texcoord = { 1.0f,0.0f };
	vertexData[5].position = { 1.0f,1.0f,0.0f,1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };
	for (UINT i = 0; i < 6; i++) {
		vertexData[i].normal = { 0.0f,0.0f,-1.0f };
	}
	//インデックスデータ
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
	//マテリアルデータ
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->lightingKind = NoneLighting;
	materialData->uvTransform = MakeIdentity4x4();
	materialData->isTexture = true;
	//座標変換行列データ
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	//入力されたファイルパスからテクスチャデータのインデックスを受け取る
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);


}

void Sprite::Update()
{
	//トランスフォームの情報を作る
	Transform transform;
	transform.translate = { position.x,position.y,0.0f };
	transform.rotate = { 0.0f,0.0f,rotation };
	transform.scale = { size.x,size.y,1.0f };

	//レンダリングパイプライン
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, (float)WinApp::kClientWidth, (float)WinApp::kClientHeight, 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;
}

void Sprite::Draw()
{
	//頂点バッファービューを設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//インデックスバッファービューを設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);

	//マテリアルCBufferの場所を設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//座標変換行列CBufferの場所を設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	//SRVのDescriptorTableの先頭を設定
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex));

	//描画
	spriteCommon_->GetDirectXCommon()->GetCommandList()->DrawInstanced(6, 1, 0, 0);

}
