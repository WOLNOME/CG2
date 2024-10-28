#include "Sprite.h"
#include "SpriteCommon.h"
#include "WinApp.h"
#include "TextureManager.h"
#include "DirectXCommon.h"

void Sprite::Initialize(const std::string& textureFilePath)
{
	//リソースを作る
	vertexResource = SpriteCommon::GetInstance()->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::VertexData) * 4);
	indexResource = SpriteCommon::GetInstance()->GetDirectXCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
	directionalLightResource = SpriteCommon::GetInstance()->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::DirectionalLight));
	materialResource = SpriteCommon::GetInstance()->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::Material));
	transformationMatrixResource = SpriteCommon::GetInstance()->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::TransformationMatrix));

	//バッファービューを作る
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(Struct::VertexData) * 4;
	vertexBufferView.StrideInBytes = sizeof(Struct::VertexData);
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	//リソースにデータをセット
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	///データに書き込む
	//頂点データ
	//左下
	vertexData[0].position = { 0.0f,1.0f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	//左上
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	//右下
	vertexData[2].position = { 1.0f,1.0f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	//右上
	vertexData[3].position = { 1.0f,0.0f,0.0f,1.0f };
	vertexData[3].texcoord = { 1.0f,0.0f };
	for (UINT i = 0; i < 4; i++) {
		vertexData[i].normal = { 0.0f,0.0f,-1.0f };
	}
	//インデックスデータ(当てられてる数字はVertexDataの要素)
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
	//平行光源用データ
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
	//マテリアルデータ
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->lightingKind = NoneLighting;
	materialData->uvTransform = MakeIdentity4x4();
	materialData->isTexture = true;
	//座標変換行列データ
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	//テクスチャファイルパスを受け取る
	textureFilePath_ = textureFilePath;
	//スプライトのサイズを本来の画像のサイズに合わせる
	AdjustTextureSize();
}

void Sprite::Update()
{
	//トランスフォームの情報を作る
	Transform transform;
	transform.translate = { position.x,position.y,0.0f };
	transform.rotate = { 0.0f,0.0f,rotation };
	transform.scale = { size.x,size.y,1.0f };

	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	//左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
	}
	//上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
	}

	//指定したテクスチャ番号のメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);
	float tex_left = textureLeftTop.x / metadata.width;
	float tex_right = (textureLeftTop.x + textureSize.x) / metadata.width;
	float tex_top = textureLeftTop.y / metadata.height;
	float tex_bottom = (textureLeftTop.y + textureSize.y) / metadata.height;

	//データ書き換え処理
	vertexData[0].position = { left,bottom,0.0f,1.0f };
	vertexData[1].position = { left,top,0.0f,1.0f };
	vertexData[2].position = { right,bottom,0.0f,1.0f };
	vertexData[3].position = { right,top,0.0f,1.0f };

	vertexData[0].texcoord = { tex_left,tex_bottom };
	vertexData[1].texcoord = { tex_left,tex_top };
	vertexData[2].texcoord = { tex_right,tex_bottom };
	vertexData[3].texcoord = { tex_right,tex_top };



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
	SpriteCommon::GetInstance()->GetDirectXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//インデックスバッファービューを設定
	SpriteCommon::GetInstance()->GetDirectXCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);

	//マテリアルCBufferの場所を設定
	SpriteCommon::GetInstance()->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//座標変換行列CBufferの場所を設定
	SpriteCommon::GetInstance()->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	//SRVのDescriptorTableの先頭を設定
	SpriteCommon::GetInstance()->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));

	//平行光源の設定
	SpriteCommon::GetInstance()->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

	//描画
	SpriteCommon::GetInstance()->GetDirectXCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Sprite::AdjustTextureSize()
{
	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);

	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);
	//画像サイズをテクスチャサイズに合わせる
	size = textureSize;
}
