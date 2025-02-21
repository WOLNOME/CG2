#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <string>
#include <cstdint>
#include "WorldTransform.h"
#include "BaseCamera.h"
#include "MyMath.h"

class Skybox {
public:
	// 初期化メソッド
	void Initialize();
	void Update();
	void Draw(WorldTransform& worldTransform, const  BaseCamera& camera, int32_t textureHandle);

private:
	// 頂点データ
	struct VertexData {
		Vector4 position;
	};

	// スカイボックスリソース
	struct SkyboxResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		VertexData* vertexData;
		uint32_t vertexNum;
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		uint32_t* indexData;
		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
	};

	// スカイボックスリソース作成関数
	SkyboxResource MakeSkyboxResource();

private:
	SkyboxResource skyboxResource_;
};