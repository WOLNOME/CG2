#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>
#include <array>

class TextTextureRender {
private://コンストラクタ等の隠蔽
	static TextTextureRender* instance;

	TextTextureRender() = default;//コンストラクタ隠蔽
	~TextTextureRender() = default;//デストラクタ隠蔽
	TextTextureRender(TextTextureRender&) = delete;//コピーコンストラクタ封印
	TextTextureRender& operator=(TextTextureRender&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static TextTextureRender* GetInstance();
public:
	//初期化
	void Initialize();
	//終了
	void Finalize();

	//描画前処理
	void PreDraw();
	//描画後処理
	void PostDraw();

	//コマンドの準備
	void ReadyNextCommand();


private://生成系メンバ関数
	void InitCommand();
	void InitViewPort();
	void InitScissorRect();

public://getter
	//コマンドアロケーター
	ID3D12CommandAllocator* GetCommandAllocator() const { return commandAllocator.Get(); }
	//コマンドリスト
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }

private://メンバ変数
	//コマンドアロケーター
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	//コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};

};

