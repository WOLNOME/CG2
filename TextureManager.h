#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
//DirectXTex
#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon;

class TextureManager
{
private://コンストラクタ等の隠蔽
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;
public:
	//シングルトンインスタンスの取得
	static TextureManager* GetInstance();
	//初期化
	void Initialize(DirectXCommon* dxCommon);
	//終了
	void Finalize();

	//テクスチャファイル読み込み
	 void LoadTexture(const std::string& filePath);
	 //SRVインデックスの開始番号
	 uint32_t GetTextureIndexByFilePath(const std::string& filePath);
	 //GPUハンドルを取得
	 D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

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

