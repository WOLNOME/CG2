#include "ImGuiManager.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "DirectXCommon.h"
#include "WinApp.h"
#include "SrvManager.h"
#include <cstdint>

void ImGuiManager::Initialize(DirectXCommon* dxCommon, WinApp* winApp, SrvManager* srvManager)
{
	//インスタンス設定
	dxCommon_ = dxCommon;
	winApp_ = winApp;
	srvManager_ = srvManager;

	//ImGuiのコンテキストを生成
	ImGui::CreateContext();
	//ImGuiのスタイルを設定(クラシック)
	ImGui::StyleColorsClassic();

	//Win32用初期化関数
	ImGui_ImplWin32_Init(winApp_->GetHwnd());
	//使用するSRVのインデックスを受け取る
	uint32_t index = srvManager_->Allocate();
	//dx12用初期化関数
	ImGui_ImplDX12_Init(
		dxCommon_->GetDevice(),
		static_cast<int>(dxCommon_->GetBackBufferCount()),
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvManager_->GetDescriptorHeap(),
		srvManager_->GetCPUDescriptorHandle(index),
		srvManager_->GetGPUDescriptorHandle(index)
	);
}

void ImGuiManager::Finalize()
{
	//後始末
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
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
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	//デスクリプタヒープの配列をセットするコマンド
	ID3D12DescriptorHeap* ppHeaps[] = { srvManager_->GetDescriptorHeap() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//描画コマンドを発行
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}
