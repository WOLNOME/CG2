#include "Sprite.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "TextureManager.h"
#include "TextTextureManager.h"
#include "SpriteCommon.h"

void Sprite::Initialize() {
	//リソースを作る
	vertexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * 4);
	indexResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * 6);
	materialResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
	transformationMatrixResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(TransformationMatrix));

	//バッファービューを作る
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView.StrideInBytes = sizeof(VertexData);
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
	//マテリアルデータ
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->uvTransform = MyMath::MakeIdentity4x4();
	materialData->isTexture = true;
	//座標変換行列データ
	transformationMatrixData->WVP = MyMath::MakeIdentity4x4();
	transformationMatrixData->World = MyMath::MakeIdentity4x4();

}

void Sprite::Draw(uint32_t _textureHandle) {
	//トランスフォームの情報を作る
	TransformEuler transform;
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
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(_textureHandle);
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
	Matrix4x4 worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MyMath::MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MyMath::MakeOrthographicMatrix(0.0f, 0.0f, (float)WinApp::kClientWidth, (float)WinApp::kClientHeight, 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = MyMath::Multiply(worldMatrix, MyMath::Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;


	//頂点バッファービューを設定
	MainRender::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//インデックスバッファービューを設定
	MainRender::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView);

	//マテリアルCBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//座標変換行列CBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	//SRVのDescriptorTableの先頭を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(_textureHandle));

	//描画
	MainRender::GetInstance()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Sprite::Draw(Handle _textTextureHandle) {
	//トランスフォームの情報を作る
	TransformEuler transform;
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

	//テクスチャのサイズからtexcoordを計算
	float tex_left = textureLeftTop.x / TextTextureManager::GetInstance()->GetTextureWidth(_textTextureHandle);
	float tex_right = (textureLeftTop.x + textureSize.x) / TextTextureManager::GetInstance()->GetTextureWidth(_textTextureHandle);
	float tex_top = textureLeftTop.y / TextTextureManager::GetInstance()->GetTextureHeight(_textTextureHandle);
	float tex_bottom = (textureLeftTop.y + textureSize.y) / TextTextureManager::GetInstance()->GetTextureHeight(_textTextureHandle);

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
	Matrix4x4 worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MyMath::MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MyMath::MakeOrthographicMatrix(0.0f, 0.0f, (float)WinApp::kClientWidth, (float)WinApp::kClientHeight, 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = MyMath::Multiply(worldMatrix, MyMath::Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;


	//頂点バッファービューを設定
	MainRender::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	//インデックスバッファービューを設定
	MainRender::GetInstance()->GetCommandList()->IASetIndexBuffer(&indexBufferView);

	//マテリアルCBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//座標変換行列CBufferの場所を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());

	//SRVのDescriptorTableの先頭を設定
	MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextTextureManager::GetInstance()->GetSrvHandleGPU(_textTextureHandle));

	//描画
	MainRender::GetInstance()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}

void Sprite::AdjustTextureSize(uint32_t _textureHandle) {
	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(_textureHandle);

	textureSize.x = static_cast<float>(metadata.width);
	textureSize.y = static_cast<float>(metadata.height);
	//画像サイズをテクスチャサイズに合わせる
	size = textureSize;
}

void Sprite::AdjustTextureSize(Handle _textTextureHandle) {
	textureSize.x = static_cast<float>(TextTextureManager::GetInstance()->GetTextureWidth(_textTextureHandle));
	textureSize.y = static_cast<float>(TextTextureManager::GetInstance()->GetTextureHeight(_textTextureHandle));
	//画像サイズをテクスチャサイズに合わせる
	size = textureSize;
}
