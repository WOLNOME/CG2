#include "TextWrite.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "GPUDescriptorManager.h"
#include "RTVManager.h"
#include "MainRender.h"
#include "D2DRender.h"
#include "TextWriteManager.h"
#include "ImGuiManager.h"

TextWrite::~TextWrite() {
	//マネージャーの登録を外す
	TextWriteManager::GetInstance()->CancelRegistration(name_);
}

void TextWrite::Initialize(const std::string& name) {
	name_ = name;

	//システムの初期化
	CreateTextureResource();
	CreateWrappedTextureResource();


	//変数の初期化
	text_ = L"";
	font_ = Font::Meiryo;
	fontName_ = ReturnFontName(font_);
	fontStyle_ = FontStyle::Normal;
	fontFaceKey_ = TextWriteManager::GetInstance()->GenerateFontKey(fontName_, fontStyle_);
	color_ = { 1.0f,1.0f,1.0f,1.0f };
	position_ = { 0.0f,0.0f };
	width_ = 100.0f;
	height_ = 100.0f;
	size_ = 32.0f;

	edgeName_ = name_ + "_Edge";
	edgeColor_ = { 0.0f,0.0f,0.0f,1.0f };
	edgeStrokeWidth_ = 10.0f;
	edgeSlideRate_ = { 0.0f,0.0f };
	isEdgeDisplay_ = false;

	//マネージャーに登録する
	TextWriteManager::GetInstance()->Registration(this);
}

void TextWrite::DebugWithImGui() {
#ifdef _DEBUG
	ImGui::Begin(("TextDebugger : " + name_).c_str());
	//テキスト
	if (ImGui::CollapsingHeader("Text")) {
		//テキストの座標
		Vector2 position = position_;
		ImGui::DragFloat2("position", &position.x, 1.0f);
		SetPosition(position);
		//フォント
		Font currentFont = font_;
		const char* fontNames[] = {
			"Meiryo",
			"YuGothic",
			"YuMincho",
			"UDDegitalN_B",
			"UDDegitalN_R",
			"UDDegitalNK_B",
			"UDDegitalNK_R",
			"UDDegitalNP_B",
			"UDDegitalNP_R",
			"OnionScript"
		};
		int fontIndex = static_cast<int>(currentFont);
		if (ImGui::Combo("Font", &fontIndex, fontNames, IM_ARRAYSIZE(fontNames))) {
			//フォントを変更
			currentFont = static_cast<Font>(fontIndex);
			SetFont(currentFont);
		}
		//フォントのスタイル
		FontStyle currentFontStyle = fontStyle_;

		//テキストのサイズ
		ImGui::DragFloat("size", &size_, 0.1f, 10.0f, 80.0f);
		SetSize(size_);
		//テキストのカラー
		ImGui::ColorEdit4("color", &color_.x);
		SetColor(color_);
	}

	//アウトライン
	if (ImGui::CollapsingHeader("Outline")) {
		//表示切り替え
		bool isDisplay = GetIsEdgeDisplay();
		ImGui::Checkbox("display", &isDisplay);
		SetIsEdgeDisplay(isDisplay);
		//アウトラインのカラー
		ImGui::ColorEdit4("colorEdge", &edgeColor_.x);
		SetEdgeColor(edgeColor_);
		//アウトラインの幅
		ImGui::SliderFloat("strokeWidth", &edgeStrokeWidth_, 1.0f, 30.0f);
		SetEdgeStrokeWidth(edgeStrokeWidth_);
		//アウトラインのスライド量
		ImGui::SliderFloat2("slideRate", &edgeSlideRate_.x, -0.005f, 0.005f);
		SetEdgeSlideRate(edgeSlideRate_);
	}

	ImGui::End();
#endif // _DEBUG
}

void TextWrite::CreateTextureResource() {
	const auto directXCommon = DirectXCommon::GetInstance();
	const auto rtvManager = RTVManager::GetInstance();
	const auto gpuDescriptorManager = GPUDescriptorManager::GetInstance();
	const auto mainrender = MainRender::GetInstance();
	//<!> RTV,SRVともにRenderTexture用の設定を利用する。

	//RTVのインデックスを取得
	rtvIndex_ = rtvManager->Allocate();
	//テクスチャリソース用のRTVを作成
	textureResource_ = directXCommon->CreateRenderTextureResource(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(0, 0, 0, 0));
	rtvManager->CreateRTVDescriptor(rtvIndex_, textureResource_.Get());

	//SRVのインデックスを取得
	srvIndex_ = gpuDescriptorManager->Allocate();
	//テクスチャリソース用のSRVを作成
	gpuDescriptorManager->CreateSRVforRenderTexture(srvIndex_, textureResource_.Get());
}

void TextWrite::CreateWrappedTextureResource() {
	HRESULT hr;
	const auto winapp = WinApp::GetInstance();
	const auto directXCommon = DirectXCommon::GetInstance();
	const auto rtvManager = RTVManager::GetInstance();
	const auto gpuDescriptorManager = GPUDescriptorManager::GetInstance();
	const auto mainrender = MainRender::GetInstance();
	const auto d2dRender = D2DRender::GetInstance();

	//DirectWriteの描画先の生成
	D3D11_RESOURCE_FLAGS resourceFlags = { D3D11_BIND_RENDER_TARGET };
	const UINT dpi = GetDpiForWindow(winapp->GetHwnd());
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), static_cast<float>(dpi), static_cast<float>(dpi));

	//D2Dで使える用のリソースを生成
	ComPtr < ID3D11Resource> wrappedTextureResource = nullptr;
	//ID3D11Resourceの生成
	hr = d2dRender->GetD3D11On12Device()->CreateWrappedResource(textureResource_.Get(), &resourceFlags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, IID_PPV_ARGS(&wrappedTextureResource));
	assert(SUCCEEDED(hr));
	//IDXGISurfaceの生成
	ComPtr<IDXGISurface> dxgiSurface = nullptr;
	hr = wrappedTextureResource.As(&dxgiSurface);
	assert(SUCCEEDED(hr));
	//ID2D1Bitmap1の生成
	ComPtr<ID2D1Bitmap1> d2dRenderTarget = nullptr;
	hr = d2dRender->GetD2DDeviceContext()->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, &d2dRenderTarget);
	assert(SUCCEEDED(hr));

	//作成した変数をメンバ変数に格納
	wrappedTextureResource_ = wrappedTextureResource;
	d2dRenderTarget_ = d2dRenderTarget;
}

void TextWrite::SetParam(const Vector2& position, const Font& font, float size, const Vector4& color) {
	SetPosition(position);
	SetFont(font);
	SetSize(size);
	SetColor(color);
	//ポジションから幅と高さを計算(初期化時限定)
	width_ = WinApp::GetInstance()->kClientWidth - position_.x;
	height_ = WinApp::GetInstance()->kClientHeight - position_.y;
}

void TextWrite::SetFont(const Font& font) {
	//フォントをセット
	font_ = font;
	//フォント名をセット
	fontName_ = ReturnFontName(font_);
	//フォントフェイスキーの更新
	fontFaceKey_ = TextWriteManager::GetInstance()->GenerateFontKey(fontName_, fontStyle_);
	//フォント情報をマネージャーにセット
	TextWriteManager::GetInstance()->EditTextFormat(name_, fontName_, size_);
}

void TextWrite::SetFontStyle(const FontStyle& fontStyle) {
	fontStyle_ = fontStyle;
	//フォントフェイスキーの更新
	fontFaceKey_ = TextWriteManager::GetInstance()->GenerateFontKey(fontName_, fontStyle_);
	//フォント情報をマネージャーにセット
	TextWriteManager::GetInstance()->EditTextFormat(name_, fontName_, size_);
}

void TextWrite::SetSize(float size) {
	//サイズをセット
	size_ = size;
	//サイズ情報をマネージャーにセット
	TextWriteManager::GetInstance()->EditTextFormat(name_, fontName_, size_);
}

void TextWrite::SetColor(const Vector4& color) {
	//色をセット
	color_ = color;
	//色情報をマネージャーにセット
	TextWriteManager::GetInstance()->EditSolidColorBrash(name_, color_);
}

void TextWrite::SetEdgeParam(const Vector4& color, float strokeWidth, const Vector2& slideRate, bool isDisplay) {
	SetEdgeColor(color);
	SetEdgeStrokeWidth(strokeWidth);
	SetEdgeSlideRate(slideRate);
	SetIsEdgeDisplay(isDisplay);
}

void TextWrite::SetEdgeColor(const Vector4& color) {
	//色をセット
	edgeColor_ = color;
	//色をマネージャーにセット
	TextWriteManager::GetInstance()->EditSolidColorBrash(edgeName_, edgeColor_);
}

const std::wstring& TextWrite::ReturnFontName(const Font& font) {
	static const std::wstring meiryo = L"Meiryo";
	static const std::wstring yugothic = L"Yu Gothic";
	static const std::wstring yumincho = L"Yu Mincho";
	static const std::wstring udDegitalN_B = L"UD Digi Kyokasho N-B";
	static const std::wstring udDegitalN_R = L"UD Digi Kyokasho N-R";
	static const std::wstring udDegitalNK_B = L"UD Digi Kyokasho NK-B";
	static const std::wstring udDegitalNK_R = L"UD Digi Kyokasho NK-R";
	static const std::wstring udDegitalNP_B = L"UD Digi Kyokasho NP-B";
	static const std::wstring udDegitalNP_R = L"UD Digi Kyokasho NP-R";
	static const std::wstring onionScript = L"Tamanegi Kaisho Geki FreeVer 7";

	static const std::wstring empty = L"";

	switch (font) {
	case Font::Meiryo:
		return meiryo;
	case Font::YuGothic:
		return yugothic;
	case Font::YuMincho:
		return yumincho;
	case Font::UDDegitalN_B:
		return udDegitalN_B;
	case Font::UDDegitalN_R:
		return udDegitalN_R;
	case Font::UDDegitalNK_B:
		return udDegitalNK_B;
	case Font::UDDegitalNK_R:
		return udDegitalNK_R;
	case Font::UDDegitalNP_B:
		return udDegitalNP_B;
	case Font::UDDegitalNP_R:
		return udDegitalNP_R;
	case Font::OnionScript:
		return onionScript;
	default:
		return empty;
	}
}

void TextWrite::WriteOnManager() {
	//描画処理
	TextWriteManager::GetInstance()->WriteTextOnD2D(name_);
}
