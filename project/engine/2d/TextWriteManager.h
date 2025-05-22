#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "D2DRender.h"
#include <Vector4.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#pragma comment(lib, "dwrite.lib")

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class TextWrite;
enum class FontStyle;

class TextWriteManager {
private://コンストラクタ等の隠蔽
	static TextWriteManager* instance;

	TextWriteManager() = default;//コンストラクタ隠蔽
	~TextWriteManager() = default;//デストラクタ隠蔽
	TextWriteManager(TextWriteManager&) = delete;//コピーコンストラクタ封印
	TextWriteManager& operator=(TextWriteManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static TextWriteManager* GetInstance();
public:

	void Initialize();
	void Finalize();

	//個別クラスを登録
	void Registration(TextWrite* piece);
	//個別クラスの登録を解除
	void CancelRegistration(const std::string& key);

	//名前を決める関数
	std::string GenerateName(const std::string& name);

	//フォントキー作成用関数
	std::string GenerateFontKey(const std::wstring& fontName, const FontStyle& style);

private:
	///=======================
	/// 初期化時処理
	///=======================

	void CreateIDWriteFactory();
	void CreateFontFile();

public:
	///=======================
	/// 描画前準備
	///=======================
	
	void EditSolidColorBrash(const std::string& key, const Vector4& color) noexcept;
	void EditTextFormat(const std::string& key, const std::wstring& fontName, const float fontSize) noexcept;
	///=======================
	/// 描画処理
	///=======================

	//D2Dでの文字列描画
	void WriteTextOnD2D(const std::string& key);
	//D3D12でのデコレーション描画
	void DrawDecorationOnD3D12(const std::string& key);


private:
	//省略変数
	WinApp* winapp = WinApp::GetInstance();
	DirectXCommon* dxcommon = DirectXCommon::GetInstance();
	MainRender* mainrender = MainRender::GetInstance();
	D2DRender* d2drender = D2DRender::GetInstance();

	//保存用変数
	ComPtr<IDWriteFactory8> directWriteFactory = nullptr;
	ComPtr<IDWriteFontCollection1> dwriteFontCollection = nullptr;
	
	//各フォントで保持しておく項目
	std::unordered_map<std::string, ComPtr<IDWriteFontFace3>> fontFaceMap;

	//各テキストで保持しておく項目(作成及び編集にデバイス等の情報が必要なため、Managerで保持)
	std::unordered_map<std::string, ComPtr<ID2D1SolidColorBrush>> solidColorBrushMap;
	std::unordered_map<std::string, ComPtr<IDWriteTextFormat>> textFormatMap;
	std::unordered_map<std::string, std::pair<ComPtr<ID2D1PathGeometry>, ComPtr<ID2D1GeometrySink>>> pathGeometryMap;

	//テキストライトコンテナ
	std::unordered_map<std::string, TextWrite*> textWriteMap;

};

///フォントを追加する時の注意点
//1. ttfもしくはttcファイルをResourcesフォルダ内のfontsフォルダに入れる
//2. TextWrite.hのFont列挙型に新たに追加する。
//3. TextWrite.cppのReturnFontName関数にfontファイル名を新たに追加する(wstringを使っているが、あとでstringに直す処理を挟むので日本語禁止)
//4. TextWrite.cppのDebugWithImGui関数のフォント欄に新たに追加する
//5. TextWriteManager.cppのDrawOutline関数内のアウトラインの位置計算に新たに追加する。
