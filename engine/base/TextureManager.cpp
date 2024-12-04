#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "StringUtility.h"

TextureManager* TextureManager::instance = nullptr;
//Imguiで0番を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Initialize()
{
	//SRVの数と回数
	textureDatas.reserve(SrvManager::GetInstance()->kMaxSRVCount);

}

void TextureManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	//読み込み済みテクスチャを検索(重複防止)
	if (textureDatas.contains(filePath)) {
		//読み込み済みなら早期return
		return;
	}

	//テクスチャ枚数上限チェック
	assert(SrvManager::GetInstance()->CheckCanSecured());

	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathw = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathw.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップの生成
	DirectX::ScratchImage mipImages{};
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas[filePath];
	textureData.metadata = metadata;
	textureData.resource = DirectXCommon::GetInstance()->CreateTextureResource(textureData.metadata);

	//テクスチャデータの転送
	UploadTextureData(textureData.resource, mipImages);

	//テクスチャデータの要素数番号からSRVのインデックスを計算する
	textureData.srvIndex = SrvManager::GetInstance()->Allocate();

	textureData.srvHandleCPU = SrvManager::GetInstance()->GetCPUDescriptorHandle(textureData.srvIndex);
	textureData.srvHandleGPU = SrvManager::GetInstance()->GetGPUDescriptorHandle(textureData.srvIndex);

	//メタデータをもとにsrvの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVの生成
	DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

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

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
	//範囲外指定違反チェック
	assert(textureDatas[filePath].srvIndex + kSRVIndexTop < SrvManager::GetInstance()->kMaxSRVCount);

	TextureData& textureData = textureDatas[filePath];
	return textureData.metadata;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath)
{
	//読み込み済みテクスチャを検索(重複防止)
	if (textureDatas.contains(filePath)) {
		//読み込み済みなら要素番号を返す
		return textureDatas[filePath].srvIndex;
	}

	//ここに来る事は想定されていない(事前にテクスチャの読み込み必須)
	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
	//範囲外指定違反チェック
	assert(textureDatas[filePath].srvIndex + kSRVIndexTop < SrvManager::GetInstance()->kMaxSRVCount);

	TextureData& textureData = textureDatas[filePath];
	return textureData.srvHandleGPU;
}

