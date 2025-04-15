#include "D2DRender.h"
#include <cassert>

D2DRender* D2DRender::instance = nullptr;

D2DRender* D2DRender::GetInstance() {
	if (instance == nullptr) {
		instance = new D2DRender;
	}
	return instance;
}

void D2DRender::Initialize() {
	//D2DResourceの生成
	CreateD2DResources();
}

void D2DRender::Finalize() {
	delete instance;
	instance = nullptr;
}

void D2DRender::PreDraw() {
	const auto backBufferIndex = mainrender->GetSwapChain()->GetCurrentBackBufferIndex();
	const auto wrappedBackBuffer = wrappedBackBuffers[backBufferIndex];
	const auto backBufferForD2D = d2dRenderTargets[backBufferIndex];

	d3d11On12Device->AcquireWrappedResources(wrappedBackBuffer.GetAddressOf(), 1);
	d2dDeviceContext->SetTarget(backBufferForD2D.Get());
	d2dDeviceContext->BeginDraw();
}

void D2DRender::PostDraw() {
	const auto backBufferIndex = mainrender->GetSwapChain()->GetCurrentBackBufferIndex();
	const auto wrappedBackBuffer = wrappedBackBuffers[backBufferIndex];

	d2dDeviceContext->EndDraw();
	d3d11On12Device->ReleaseWrappedResources(wrappedBackBuffer.GetAddressOf(), 1);
	//描画内容の確定(スワップ可能状態に移行)
	d3d11On12DeviceContext->Flush();
}

void D2DRender::CreateD2DResources() {
	HRESULT hr;
	//ID3D11On12Deviceの生成
	ComPtr<ID3D11Device
	> d3d11Device = nullptr;
	UINT d3d11DeviceFlags = 0U;
#ifdef _DEBUG
	d3d11DeviceFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#else
	d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif // _DEBUG
	hr = D3D11On12CreateDevice(
		dxcommon->GetDevice(),
		d3d11DeviceFlags,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(dxcommon->GetAddressOfCommandQueue()),
		1,
		0,
		&d3d11Device,
		&d3d11On12DeviceContext,
		nullptr
	);
	assert(SUCCEEDED(hr));
	//D3D11->D3D11On12
	hr = d3d11Device.As(&d3d11On12Device);
	assert(SUCCEEDED(hr));
	//ID2D1Factory3の生成
	D2D1_FACTORY_OPTIONS factoryOptions{};
	factoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &factoryOptions, &d2dFactory);
	assert(SUCCEEDED(hr));
	//IDXGIDeviceの生成
	ComPtr<IDXGIDevice> dxgiDevice = nullptr;
	hr = d3d11On12Device.As(&dxgiDevice);
	assert(SUCCEEDED(hr));
	//ID2D1Device2の生成
	ComPtr<ID2D1Device2> d2dDevice = nullptr;
	hr = d2dFactory->CreateDevice(dxgiDevice.Get(), d2dDevice.ReleaseAndGetAddressOf());
	//d2dDeviceContextの生成
	hr = d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dDeviceContext.ReleaseAndGetAddressOf());
	assert(SUCCEEDED(hr));
	//DirectWriteの描画先の生成
	D3D11_RESOURCE_FLAGS resourceFlags = { D3D11_BIND_RENDER_TARGET };
	const UINT dpi = GetDpiForWindow(winapp->GetHwnd());
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), static_cast<float>(dpi), static_cast<float>(dpi));
	for (UINT i = 0U; i < mainrender->GetBackBufferCount(); ++i) {
		ComPtr<ID3D11Resource> wrappedBackBuffer = nullptr;
		//ID3D11Resourceの生成
		hr = d3d11On12Device->CreateWrappedResource(mainrender->GetSwapChainResource(i), &resourceFlags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(wrappedBackBuffer.ReleaseAndGetAddressOf()));
		assert(SUCCEEDED(hr));
		//IDXGISurfaceの生成
		ComPtr<IDXGISurface> dxgiSurface = nullptr;
		hr = wrappedBackBuffer.As(&dxgiSurface);
		assert(SUCCEEDED(hr));
		//ID2D1Bitmap1の生成
		ComPtr<ID2D1Bitmap1> d2dRenderTarget = nullptr;
		hr = d2dDeviceContext->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, &d2dRenderTarget);
		assert(SUCCEEDED(hr));

		wrappedBackBuffers.emplace_back(wrappedBackBuffer);
		d2dRenderTargets.emplace_back(d2dRenderTarget);
	}
}