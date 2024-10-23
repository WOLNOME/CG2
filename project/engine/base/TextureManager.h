#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
//DirectXTex
#include "DirectXTex.h"

class DirectXCommon;

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
	void Initialize(DirectXCommon* dxCommon);
	//終了
	void Finalize();

	//テクスチャファイル読み込み
	 void LoadTexture(const std::string& filePath);
	 //GPUハンドルを取得
	 D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);
private://非公開メンバ関数
	//テクスチャデータの転送
	void UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);
public://ゲッター
	 //SRVインデックスの開始番号
	 uint32_t GetTextureIndexByFilePath(const std::string& filePath);
	//メタデータを取得
	const DirectX::TexMetadata& GetMetaData(uint32_t textureIndex);
private:
	//テクスチャデータ
	struct TextureData
	{
		std::string filePath;
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};
public:
	//SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;
private://インスタンス
	DirectXCommon* dxCommon;
private://メンバ変数
	//テクスチャデータ
	std::vector<TextureData> textureDatas;
};

