#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include <d2d1_3.h>
#include <d3d11on12.h>
#include <wrl.h>
#include <vector>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class D2DRender {
private://コンストラクタ等の隠蔽
	static D2DRender* instance;
	D2DRender() = default;//コンストラクタ隠蔽
	~D2DRender() = default;//デストラクタ隠蔽
	D2DRender(D2DRender&) = delete;//コピーコンストラクタ封印
	D2DRender& operator=(D2DRender&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static D2DRender* GetInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();

	//描画前処理
	void PreDraw();
	//描画後処理
	void PostDraw();

public://getter
	//D3D11On12Device
	ID3D11On12Device* GetD3D11On12Device() const { return d3d11On12Device.Get(); }
	//D3D11On12DeviceContext
	ID3D11DeviceContext* GetD3D11On12DeviceContext() const { return d3d11On12DeviceContext.Get(); }
	//D2DFactory
	ID2D1Factory3* GetD2DFactory() const { return d2dFactory.Get(); }
	//D2DDeviceContext
	ID2D1DeviceContext2* GetD2DDeviceContext() const { return d2dDeviceContext.Get(); }
private:
	//D2DFactoryの生成
	void CreateD2DResources();

private://インスタンス
	WinApp* winapp = WinApp::GetInstance();
	DirectXCommon* dxcommon = DirectXCommon::GetInstance();
	MainRender* mainrender = MainRender::GetInstance();

private:
	//D3D11On12Device
	ComPtr<ID3D11On12Device> d3d11On12Device = nullptr;
	//D3D11On12DeviceContext
	ComPtr<ID3D11DeviceContext> d3d11On12DeviceContext = nullptr;
	//D2DFactory
	ComPtr<ID2D1Factory3> d2dFactory = nullptr;
	//D2DDevice
	ComPtr<ID2D1DeviceContext2> d2dDeviceContext = nullptr;
	std::vector<ComPtr<ID3D11Resource>> wrappedBackBuffers;
	std::vector<ComPtr<ID2D1Bitmap1>> d2dRenderTargets;
};

