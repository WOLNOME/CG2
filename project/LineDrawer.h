#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <cstdint>
#include <list>
#include <vector>
#include <numbers>
#include <memory>
#include "MyMath.h"

class LineDrawer
{
public://構造体
	//頂点データ
	struct VertexForVS {
		Vector4 position;
		float vertexIndex;
	};
	//座標変換行列データ(インスタンスごとに変えられるデータ)
	struct LineForGPU {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Vector4 start;
		Vector4 end;
		Vector4 color;
	};
	//線リソース作成用データ型
	struct LineResource {
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		VertexForVS* vertexData;
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
		LineForGPU* instancingData;
		Transform transform;
		D3D12_CPU_DESCRIPTOR_HANDLE SrvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE SrvHandleGPU;
	};
	//ライン情報(外から書き換える用)
	struct Line
	{
		Vector4 start;
		Vector4 end;
		Vector4 color;
	};
public://メンバ関数
	LineDrawer();
	~LineDrawer();

	//初期化
	void Initialize();
	void Draw();

	//ライン作成関数(ハンドルを返却)
	void CreateLine(Vector3 start, Vector3 end, Vector4 color);

private://非公開メンバ関数
	//パーティクルリソース作成関数
	LineResource MakeLineResource();
	//インスタンシングをSRVにセット
	void SettingSRV();
private://インスタンス
private://メンバ変数
	//一つのインスタンスで表示できる線の最大数
	static const size_t kNumMaxLine_ = 512;
	//インスタンシング用書き換え情報
	std::list<Line> lines_;
	//パーティクル用リソース
	LineResource lineResource_;
	
	//カメラトランスフォーム
	Transform cameraTransform = {
		{1.0f,1.0f,1.0f},
		{std::numbers::pi_v<float> / 3.0f,std::numbers::pi_v<float>,0.0f},
		{0.0f,23.0f,10.0f}
	};

};