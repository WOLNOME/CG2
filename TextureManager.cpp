#include "TextureManager.h"
#include "DirectXCommon.h"
#include "StringUtility.h"

TextureManager* TextureManager::instance = nullptr;
//Imguiで0版を使用するため、1番から使用
uint32_t TextureManager::kSRVIndexTop = 1;

TextureManager* TextureManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Initialize(DirectXCommon* dxCommon)
{
	//SRVの数と回数
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
	//dxCommonインスタンスの取得
	this->dxCommon = dxCommon;

}

void TextureManager::Finalize()
{
	delete instance;
	instance = nullptr;
}

void TextureManager::LoadTexture(const std::string& filePath)
{
	//読み込み済みテクスチャを検索(重複防止)
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		//読み込み済みなら早期return
		return;
	}
	//テクスチャ枚数上限チェック
	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

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

	//テクスチャデータを追加
	textureDatas.resize(textureDatas.size() + 1);
	//追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas.back();
	textureData.filePath = filePath;
	textureData.metadata = metadata;
	textureData.resource = dxCommon->CreateTextureResource(textureData.metadata);

	//テクスチャデータの要素数番号からSRVのインデックスを計算する
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

	textureData.srvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(srvIndex);

	//メタデータをもとにsrvの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath)
{
	//読み込み済みテクスチャデータ検索
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(),
		[&](TextureData& textureData) {return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		//読み込み済みなら要素番号を返す
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
	//範囲外指定違反チェック
	assert(textureIndex + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

	TextureData& textureData = textureDatas.at(textureIndex);
	return textureData.srvHandleGPU;


}
