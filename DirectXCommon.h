#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"


class DirectXCommon
{
public:
	void Initialize(WinApp* winApp);

private://インスタンス
	//WindowsAPI
	WinApp* winApp_ = nullptr;
private://メンバ変数
	//DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

};

