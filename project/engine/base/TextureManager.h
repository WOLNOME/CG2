#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <array>
#include <optional>
//DirectXTex
#include "DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

class TextureManager
{
private://コンストラクタ等の隠蔽
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;
public://公開メンバ関数
	//シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	//初期化
	void Initialize();
	//終了
	void Finalize();

	//テクスチャファイル読み込み
	uint32_t LoadTexture(const std::string& filePath);
private://非公開メンバ関数
	//テクスチャデータの転送
	ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);
public://ゲッター
	//メタデータを取得
	const DirectX::TexMetadata& GetMetaData(uint32_t textureHandle);
	//SRVインデックスの取得
	uint32_t GetSrvIndex(uint32_t textureHandle);
	//GPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureHandle);
private:
	//テクスチャデータ
	struct TextureData
	{
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		std::string textureName;
	};
public:
	//テクスチャデータの確保数
	static const uint32_t kNumTextureData = 512;

private://インスタンス

private://メンバ変数
	//テクスチャデータコンテナ(要素数がテクスチャハンドル)
	std::array<std::optional<TextureData>, kNumTextureData> textureDatas;
	//ディレクトリパス
	std::string directoryPath_;

};

