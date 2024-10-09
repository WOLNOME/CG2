#include "Sprite.h"
#include "SpriteCommon.h"

void Sprite::Initialize(SpriteCommon* spriteCommon)
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
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	///データに書き込む
	//頂点データ
	vertexData[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[3].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[3].texcoord = { 0.0f,0.0f };
	vertexData[4].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexData[4].texcoord = { 1.0f,0.0f };
	vertexData[5].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };
	for (UINT i = 0; i < 6; i++) {
		vertexData[i].normal = { 0.0f,0.0f,-1.0f };
	}
	//インデックスデータ
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
	//マテリアルデータ
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataSprite->lightingKind = NoneLighting;
	materialDataSprite->uvTransform = MakeIdentity4x4();
	materialDataSprite->isTexture = true;
	//座標変換行列データ
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	/////仮の処理(いらなくなったら消す)
	//テクスチャの設定
	//Textureを読んで転送する
	DirectX::ScratchImage mipImages = DirectXCommon::LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResorce = spriteCommon_->GetDirectXCommon()->CreateTextureResource(metadata);
	spriteCommon_->GetDirectXCommon()->UploadTextureData(textureResorce.Get(), mipImages);
	//metadataをもとにSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVを作成するDescriptorHeapの場所を決める
	textureSrvHandleCPU = spriteCommon_->GetDirectXCommon()->GetSRVCPUDescriptorHandle(spriteCommon_->GetDirectXCommon()->GetSRVSite());
	textureSrvHandleGPU = spriteCommon_->GetDirectXCommon()->GetSRVGPUDescriptorHandle(spriteCommon_->GetDirectXCommon()->GetSRVSite());
	//SRV配置をインクリメント
	spriteCommon_->GetDirectXCommon()->SRVSiteIncrement();
	//SRVの生成
	spriteCommon_->GetDirectXCommon()->GetDevice()->CreateShaderResourceView(textureResorce.Get(), &srvDesc, textureSrvHandleCPU);

}

void Sprite::Update()
{
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
	spriteCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

	//描画
	spriteCommon_->GetDirectXCommon()->GetCommandList()->DrawInstanced(6, 1, 0, 0);

}
