#include "TextWrite.h"
#include "TextWriteManager.h"
#include "WinApp.h"
#include "ImGuiManager.h"

TextWrite::~TextWrite() {
	//マネージャーの登録を外す
	TextWriteManager::GetInstance()->CancelRegistration(name_);
}

void TextWrite::Initialize(const std::string& name) {
	name_ = name;

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
	TextWriteManager::GetInstance()->WriteText(name_);
}
