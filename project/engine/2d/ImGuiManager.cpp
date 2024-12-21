#include "ImGuiManager.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "SrvManager.h"
#include <cstdint>

ImGuiManager* ImGuiManager::instance = nullptr;

ImGuiManager* ImGuiManager::GetInstance()
{
	if (instance == nullptr) {
		instance = new ImGuiManager;
	}
	return instance;
}

void ImGuiManager::Initialize()
{
	//ImGuiのコンテキストを生成
	ImGui::CreateContext();
	//ImGuiのスタイルを設定(クラシック)
	ImGui::StyleColorsClassic();

	//Win32用初期化関数
	ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHwnd());
	//使用するSRVのインデックスを受け取る
	uint32_t index = SrvManager::GetInstance()->Allocate();
	//dx12用初期化関数
	ImGui_ImplDX12_Init(
		DirectXCommon::GetInstance()->GetDevice(),
		static_cast<int>(MainRender::GetInstance()->GetBackBufferCount()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		SrvManager::GetInstance()->GetDescriptorHeap(),
		SrvManager::GetInstance()->GetCPUDescriptorHandle(index),
		SrvManager::GetInstance()->GetGPUDescriptorHandle(index)
	);
}

void ImGuiManager::Finalize()
{
	//後始末
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	//シングルトン解放
	delete instance;
	instance = nullptr;
}

void ImGuiManager::Begin()
{
	//ImGuiフレーム開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::End()
{
	//描画前準備
	ImGui::Render();
}

void ImGuiManager::Draw()
{
	ID3D12GraphicsCommandList* commandList = MainRender::GetInstance()->GetCommandList();

	//デスクリプタヒープの配列をセットするコマンド
	ID3D12DescriptorHeap* ppHeaps[] = { SrvManager::GetInstance()->GetDescriptorHeap() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}
