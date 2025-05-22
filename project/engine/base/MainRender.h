#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <cstdint>
#include <array>

class MainRender
{
private://コンストラクタ等の隠蔽
	static MainRender* instance;

	MainRender() = default;//コンストラクタ隠蔽
	~MainRender() = default;//デストラクタ隠蔽
	MainRender(MainRender&) = delete;//コピーコンストラクタ封印
	MainRender& operator=(MainRender&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static MainRender* GetInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();

	//オブジェクト描画前処理
	void PreObjectDraw();
	//ImGui描画前処理
	void PreImGuiDraw();
	//描画後処理
	void PostDraw();
	//画面切り替え処理
	void ExchangeScreen();

	//コマンドの準備
	void ReadyNextCommand();
private://生成系メンバ関数
	void InitCommand();
	void GenerateSwapChain();
	void GenerateDepthBuffer();
	void InitRenderTargetView();
	void InitDepthStencilView();
	void InitViewPort();
	void InitScissorRect();

public://公開メンバ変数

public://ゲッター
	//コマンドアロケーター
	ID3D12CommandAllocator* GetCommandAllocator() const { return commandAllocator.Get(); }
	//コマンドリスト
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
	//バックバッファの数を取得
	size_t GetBackBufferCount()const { DXGI_SWAP_CHAIN_DESC1 desc{}; swapChain->GetDesc1(&desc); return desc.BufferCount; }
	//スワップチェーン
	IDXGISwapChain4* GetSwapChain()const { return swapChain.Get(); }
	//スワップチェーンのリソース
	ID3D12Resource* GetSwapChainResource(uint32_t index)const { return swapChainResources[index].Get(); }
	//DSVインデックス(オフスクで使うため)
	uint32_t GetDSVIndex()const { return dsvIndex; }

private://メンバ変数
	//コマンドアロケーター
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	//スワップチェーンから引っ張て来たリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;
	//深度描画用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;
	//RTVインデックス
	std::array<uint32_t, 2> rtvIndices;
	//DSVインデックス
	uint32_t dsvIndex = 0;
	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};

};

