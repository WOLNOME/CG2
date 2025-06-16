#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "MainRender.h"
#include "StringUtility.h"
#include <iostream>
#include <filesystem>

TextureManager* TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager;
	}
	return instance;
}

void TextureManager::Initialize() {
	//ディレクトリパスの登録
	directoryPath_ = "Resources/textures/";
}

void TextureManager::Finalize() {
	for (auto& textureData : textureDatas) {
		if (textureData.has_value()) {
			textureData->resource.Reset();
		}
	}
	delete instance;
	instance = nullptr;
}

uint32_t TextureManager::LoadTexture(const std::string& filePath) {
	HRESULT hr;
	//ファイルパスを分解
	std::string textureName = std::filesystem::path(filePath).filename().string();
	//読み込み済みテクスチャを検索(重複防止)
	for (size_t i = 0; i < textureDatas.size(); ++i) {
		if (textureDatas[i].has_value() && textureDatas[i]->textureName == textureName) {
			//同名のテクスチャが読み込み済みなのでテクスチャハンドルを返す
			return (uint32_t)i;
		}
	}

	//テクスチャ枚数上限チェック
	assert(SrvManager::GetInstance()->CheckCanSecured());

	//テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathw = StringUtility::ConvertString(directoryPath_ + filePath);
	//DDSファイルを読み込む
	if (filePathw.ends_with(L".dds")) {
		hr = DirectX::LoadFromDDSFile(filePathw.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
		assert(SUCCEEDED(hr));
	}
	//WICファイルを読み込む
	else {
		hr = DirectX::LoadFromWICFile(filePathw.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		assert(SUCCEEDED(hr));
	}

	//ミップマップの生成
	DirectX::ScratchImage mipImages{};
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image);
	}
	else {
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipImages);
		assert(SUCCEEDED(hr));
	}
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//追加したテクスチャデータの参照を取得する
	TextureData textureData;
	textureData.metadata = metadata;
	textureData.resource = DirectXCommon::GetInstance()->CreateTextureResource(textureData.metadata);

	{
		HRESULT hr;
		ID3D12CommandAllocator* allocator = MainRender::GetInstance()->GetCommandAllocator();
		ID3D12GraphicsCommandList* commandList = MainRender::GetInstance()->GetCommandList();
		ID3D12CommandQueue* commandQueue = DirectXCommon::GetInstance()->GetCommandQueue();

		//テクスチャデータの転送(valはこのブロック終了時まで保持される必要があるのでUploadTextureはnodiscard属性である)
		ID3D12Resource* val = UploadTextureData(textureData.resource.Get(), mipImages);

		//commandListをCloseし、commandQueue->ExecuteCommandListsを使いキックする
		hr = commandList->Close();
		assert(SUCCEEDED(hr));

		ID3D12CommandList* cmdLists[] = { commandList };
		commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		//実行を待つ
		Microsoft::WRL::ComPtr<ID3D12Fence> fence;
		hr= DirectXCommon::GetInstance()->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		assert(SUCCEEDED(hr));

		UINT64 fenceValue = 1;
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (fenceEvent == nullptr) {
			assert(false && "Failed to create fence event");
		}

		hr = commandQueue->Signal(fence.Get(), fenceValue);
		assert(SUCCEEDED(hr));

		if (fence->GetCompletedValue() < fenceValue) {
			hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
			assert(SUCCEEDED(hr));
			WaitForSingleObject(fenceEvent, INFINITE);
		}
		CloseHandle(fenceEvent);

		//実行が完了したので、allocateとcommandListをResetして次のコマンドを積めるようにする
		allocator->Reset();
		assert(SUCCEEDED(hr));
		commandList->Reset(allocator, nullptr);
		assert(SUCCEEDED(hr));
	}

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
			//キューブマップ用SRV設定
			if (metadata.IsCubemap()) {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = UINT_MAX;
				srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			}
			//通常のSRV設定
			else {
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
			}
			//SRVの生成
			if (textureDatas[i].has_value()) {
				DirectXCommon::GetInstance()->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, SrvManager::GetInstance()->GetCPUDescriptorHandle(textureDatas[i]->srvIndex));
			}
			return (uint32_t)i;
		}
	}

	//空きが見つからなかった場合のエラー処理
	assert(0 && "No available space in textureDatas!");
	return UINT32_MAX; // エラー時の特殊な値を返す
}

[[nodiscard]]		//戻り値を破棄してはならないという属性(x=関数の形でないと使えない)
ID3D12Resource* TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
	ID3D12Device* device = DirectXCommon::GetInstance()->GetDevice();
	ID3D12GraphicsCommandList* commandList = MainRender::GetInstance()->GetCommandList();

	//サブリソース列の作成
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
	ID3D12Resource* intermediateResource = DirectXCommon::GetInstance()->CreateBufferResource(intermediateSize);
	//テクスチャにデータ転送
	UpdateSubresources(commandList, texture, intermediateResource, 0, 0, UINT(subresources.size()), subresources.data());
	//Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;




}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureHandle) {
	//範囲外指定違反チェック
	assert(textureHandle < textureDatas.size() && textureDatas[textureHandle].has_value());
	assert(textureDatas[textureHandle]->srvIndex < SrvManager::GetInstance()->kMaxSRVCount);

	TextureData& textureData = *textureDatas[textureHandle];
	return textureData.metadata;
}

uint32_t TextureManager::GetSrvIndex(uint32_t textureHandle) {
	//読み込み済みテクスチャを検索(重複防止)
	if (textureDatas[textureHandle].has_value()) {
		//読み込み済みなら要素番号を返す
		return textureDatas[textureHandle]->srvIndex;
	}

	//ここに来る事は想定されていない(事前にテクスチャの読み込み必須)
	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureHandle) {
	//範囲外指定違反チェック
	assert(textureDatas[textureHandle]->srvIndex < SrvManager::GetInstance()->kMaxSRVCount);

	TextureData& textureData = *textureDatas[textureHandle];
	return SrvManager::GetInstance()->GetGPUDescriptorHandle(textureData.srvIndex);
}

