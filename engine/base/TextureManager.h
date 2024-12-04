#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <unordered_map>
//DirectXTex
#include "DirectXTex.h"

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
	 void LoadTexture(const std::string& filePath);
private://非公開メンバ関数
	//テクスチャデータの転送
	void UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);
public://ゲッター
	//メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string& filePath);
	 //SRVインデックスの取得
	 uint32_t GetSrvIndex(const std::string& filePath);
	 //GPUハンドルを取得
	 D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filePath);
private:
	//テクスチャデータ
	struct TextureData
	{
		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
	};
public:
	//SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;
private://インスタンス
	
private://メンバ変数
	//テクスチャデータ
	std::unordered_map<std::string, TextureData> textureDatas;
};

