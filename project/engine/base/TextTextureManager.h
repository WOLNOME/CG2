#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "TextTextureRender.h"
#include "D2DRender.h"
#include "Handle.h"
#include "Vector2.h"
#include "Vector4.h"
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wrl.h>
#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include <memory>

#pragma comment(lib, "dwrite.lib")

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

///=======================///
///		　　列挙型
///=======================///

//フォント
enum class Font {
	Meiryo,
	YuGothic,
	YuMincho,
	UDDegitalN_B,
	UDDegitalN_R,
	UDDegitalNK_B,
	UDDegitalNK_R,
	UDDegitalNP_B,
	UDDegitalNP_R,
	OnionScript,
};
//フォントスタイル
enum class FontStyle {
	Normal,		//通常
	Oblique,	//斜体(通常フォントをプログラムで斜体にする)
	Italic,		//斜体(フォントファイルベース)
};

///=======================///
///		　　構造体
///=======================///

//テキストのパラメータ
struct TextParam {
	std::wstring text;		//書き込むテキスト
	Font font;				//フォント
	FontStyle fontStyle;	//フォントスタイル
	float size;				//文字のサイズ
	Vector4 color;			//文字の色
};
//アウトラインのパラメータ
struct EdgeParam {
	uint32_t isEdgeDisplay;	//アウトライン表示フラグ
	float width;			//アウトラインの幅
	Vector2 slideRate;		//アウトラインのスライド量
	Vector4 color;			//アウトラインの色
};

class TextTextureManager {
private://構造体
	//テキストのリソース
	struct TextResource {
		ComPtr<ID3D12Resource> resource;
		Vector4* color;		//テキストの色(PSに送る)
		TextParam param;
		TextParam preParam;
	};

	//アウトラインのリソース
	struct EdgeResource {
		ComPtr<ID3D12Resource> resource;
		EdgeParam* param;
		EdgeParam preParam;
	};

	//各テキストテクスチャの必須項目
	struct TextTextureItem {
		ComPtr<ID3D12Resource> resource;				//テクスチャリソース
		ComPtr<ID3D12Resource> copyResource;			//コピー用テクスチャリソース
		ComPtr<ID3D11Resource> wrappedResource;			//D2D用のラップリソース
		ComPtr<ID2D1Bitmap1> d2dRenderTarget;			//D2D用のレンダーターゲット
		ComPtr<ID2D1SolidColorBrush> solidColorBrush;	//D2D用のブラシ
		ComPtr<IDWriteTextFormat> textFormat;			//DWrite用のテキストフォーマット
		TextResource textResource;						//テキストのリソース
		EdgeResource edgeResource;						//アウトラインのリソース
		uint32_t rtvIndex = 0;								//RTVインデックス
		uint32_t srvIndex = 0;								//SRVインデックス
		uint32_t srvCopyIndex = 0;							//コピーリソース用SRVインデックス

		UINT width;									//テクスチャの横幅
		UINT height;								//テクスチャの縦幅
		bool isDrawThisFrame = true;
	};

private://コンストラクタ等の隠蔽
	static TextTextureManager* instance;

	TextTextureManager() = default;//コンストラクタ隠蔽
	~TextTextureManager() = default;//デストラクタ隠蔽
	TextTextureManager(TextTextureManager&) = delete;//コピーコンストラクタ封印
	TextTextureManager& operator=(TextTextureManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static TextTextureManager* GetInstance();
public:

	void Initialize();
	void Finalize();

	//ImGuiでテキストデバッグ
	void DebugWithImGui(Handle _handle);

public:
	///=======================
	/// 外部とのやり取り
	///=======================

	//テクスチャの読み込み
	Handle LoadTextTexture(const TextParam& _textParam);
	Handle LoadTextTexture(uint32_t _id);

	//各種パラメータの編集
	void EditTextParam(Handle _handle, const TextParam& _textParam);
	void EditEdgeParam(Handle _handle, const EdgeParam& _edgeParam);

	//各パラメータの個別編集
	template <typename... Args>
	void EditTextString(Handle _handle, const std::wstring& text, Args&&... args) {
		//使用可能なハンドルかチェック
		CheckHandle(_handle);

		//テキストを生成
		textTextureMap[_handle.id].textResource.param.text = std::vformat(text, std::make_wformat_args(args...));
	}
	void EditTextFont(Handle _handle, const Font& _font);
	void EditTextFontStyle(Handle _handle, const FontStyle& _fontStyle);
	void EditTextSize(Handle _handle, const float _size);
	void EditTextColor(Handle _handle, const Vector4& _color);

	void EditIsEdgeDisplay(Handle _handle, const bool _isDisplay);
	void EditEdgeWidth(Handle _handle, const float _width);
	void EditEdgeSlideRate(Handle _handle, const Vector2& _slideRate);
	void EditEdgeColor(Handle _handle, const Vector4& _color);

	//テクスチャそのもののgetter
	const UINT GetTextureWidth(Handle _handle);
	const UINT GetTextureHeight(Handle _handle);

	//各パラメータのgetter
	const std::wstring& GetTextString(Handle _handle);
	const Font& GetTextFont(Handle _handle);
	const FontStyle& GetTextFontStyle(Handle _handle);
	const float GetTextSize(Handle _handle);
	const Vector4& GetTextColor(Handle _handle);

	const bool GetIsEdgeDisplay(Handle _handle);
	const float GetEdgeWidth(Handle _handle);
	const Vector2& GetEdgeSlideRate(Handle _handle);
	const Vector4& GetEdgeColor(Handle _handle);


	//SRV関係のgetter
	uint32_t GetSrvIndex(Handle _handle);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(Handle _handle);

private:
	void CheckHandle(Handle _handle);
	TextTextureItem CreateTextTextureItem(const TextParam& _textParam);
	ComPtr<ID2D1SolidColorBrush> CreateSolidColorBrush(const Vector4& color);
	ComPtr<IDWriteTextFormat> CreateTextFormat(const Font& _font, const FontStyle& _fontStyle, const float fontSize) noexcept;
	void TransitionState(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

private:
	///=======================
	/// 初期化時処理
	///=======================

	void GenerateIDWriteFactory();
	void GenerateFontFile();
	std::string GenerateFontKey(const std::wstring& fontName, const FontStyle& style);
	void GenerateGraphicsPipeline();

private:
	///=======================
	/// 描画前準備
	///=======================

	//描画前参照チェック関数
	void CheckAllReference();
	//テクスチャのサイズを揃える
	void ArrangeTextureSize(uint32_t _id);

public:
	///=======================
	/// 描画処理
	///=======================

	//D2Dでの文字列描画
	void WriteTextOnD2D();
	//D3D12でのデコレーション描画
	void DrawDecorationOnD3D12();

	///=======================
	/// 描画後処理
	///=======================

	//次のフレームで使える状態に遷移
	void ReadyNextResourceState();

private:
	//省略変数
	WinApp* winapp = WinApp::GetInstance();
	DirectXCommon* dxcommon = DirectXCommon::GetInstance();
	TextTextureRender* ttrender = TextTextureRender::GetInstance();
	D2DRender* d2drender = D2DRender::GetInstance();

	//マネージャ全体での保存用変数
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;			//ルートシグネチャ
	ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;	//グラフィックスパイプライン

	ComPtr<IDWriteFactory8> directWriteFactory = nullptr;
	ComPtr<IDWriteFontCollection1> dwriteFontCollection = nullptr;
	std::unordered_map<std::string, ComPtr<IDWriteFontFace3>> fontFaceMap;	//各フォントで保持しておく項目

	//テキストテクスチャのコンテナ
	std::unordered_map<uint32_t, TextTextureItem> textTextureMap;	//各テキストで保持しておく項目(作成及び
	//最新の空きインデックス
	uint32_t useIndex = 0;
	//解放済みインデックスを管理するリスト(描画前参照チェック関数によって割り当てられる)
	std::list<uint32_t> freeIndices;
	//参照カウンタ
	std::list<Handle> referenceCounter;

	//アウトラインの最大幅
	const float kMaxEdgeWidth_ = 3.0f;

};

///フォントを追加する時の注意点
//1. ttfもしくはttcファイルをResourcesフォルダ内のfontsフォルダに入れる
//2. このファイルのFont列挙型に新たに追加する。
//3. TextWrite.cppのReturnFontName関数にfontファイル名を新たに追加する(wstringを使っているが、あとでstringに直す処理を挟むので日本語禁止)
//4. TextWrite.cppのDebugWithImGui関数のフォント欄に新たに追加する
//5. TextWriteManager.cppのDrawOutline関数内のアウトラインの位置計算に新たに追加する。
