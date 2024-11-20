#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "StringUtility.h"
#include <iostream>
#include <filesystem>

TextureManager* TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Initialize(std::string directoryPath)
{
	//ディレクトリパスの登録
	directoryPath_ = directoryPath;
}

void TextureManager::Finalize()
{
	for (auto& textureData : textureDatas) {
		if (textureData.has_value()) {
			textureData->resource.Reset();
		}
	}
	delete instance;
	instance = nullptr;
}

uint32_t TextureManager::LoadTexture(const std::string& filePath)
{
	//ファイルパスを分解
	std::string textureName = std::filesystem::path(filePath).filename().string();
	//読み込み済みテクスチャを検索(重複防止)
	for (size_t i = 0; i < textureDatas.size(); ++i) {
		if (textureDatas[i].has_value() && textureDatas[i]->textureName == textureName) {
			//同名のテクスチャが読み込み済みなのでテクスチャハンドルを返す
			return i;
		}
	}

	//テクスチャ枚数上限チェック
	assert(SrvManager::GetInstance()->CheckCanSecured());

	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathw = StringUtility::ConvertString(directoryPath_ + filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathw.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	if (FAILED(hr)) {
		std::cerr << "Failed to load texture file: " << directoryPath_ + filePath
			<< ", HRESULT: " << std::hex << hr << std::endl;
		assert(false);
	}

	//ミップマップの生成
	DirectX::ScratchImage mipImages{};
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//追加したテクスチャデータの参照を取得する
	TextureData textureData;
	textureData.metadata = metadata;
	textureData.resource = DirectXCommon::GetInstance()->CreateTextureResource(textureData.metadata);

	//テクスチャデータの転送
	UploadTextureData(textureData.resource, mipImages);

	//テクスチャデータの要素数番号からSRVのインデックスを計算する
	textureData.srvIndex = SrvManager::GetInstance()->Allocate();

	//テクスチャの名前(例:ＯＯ.png)を登録
	textureData.textureName = textureName;

	//コンテナ内の空いている要素を探して登録
	for (size_t i = 0; i < textureDatas.size(); ++i) {
		if (!textureDatas[i].has_value()) {
			//空いている場所を見つけたので登録
			textureDatas[i] = textureData;

			//メタデータをもとにsrvの設定
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
			srvDesc.Format = metadata.format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
			//SRVの生成
			if (textureDatas[i].has_value()) {
				DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, SrvManager::GetInstance()->GetCPUDescriptorHandle(textureDatas[i]->srvIndex));
			}
			return i;
		}
	}

	//空きが見つからなかった場合のエラー処理
	assert(0 && "No available space in textureDatas!");
	return UINT32_MAX; // エラー時の特殊な値を返す
}

void TextureManager::UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages)
{
	//Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel)
	{
		//MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		//Textureに転送
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,
			img->pixels,
			UINT(img->rowPitch),
			UINT(img->slicePitch)
		);
		assert(SUCCEEDED(hr));
	}
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureHandle)
{
	//範囲外指定違反チェック
	assert(textureHandle < textureDatas.size() && textureDatas[textureHandle].has_value());
	assert(textureDatas[textureHandle]->srvIndex < SrvManager::GetInstance()->kMaxSRVCount);

	TextureData& textureData = *textureDatas[textureHandle];
	return textureData.metadata;
}

uint32_t TextureManager::GetSrvIndex(uint32_t textureHandle)
{
	//読み込み済みテクスチャを検索(重複防止)
	if (textureDatas[textureHandle].has_value()) {
		//読み込み済みなら要素番号を返す
		return textureDatas[textureHandle]->srvIndex;
	}

	//ここに来る事は想定されていない(事前にテクスチャの読み込み必須)
	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureHandle)
{
	//範囲外指定違反チェック
	assert(textureDatas[textureHandle]->srvIndex < SrvManager::GetInstance()->kMaxSRVCount);

	TextureData& textureData = *textureDatas[textureHandle];
	return SrvManager::GetInstance()->GetGPUDescriptorHandle(textureData.srvIndex);
}

