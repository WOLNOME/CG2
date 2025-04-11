#pragma once
#include "d2d1_3.h"
#include "dwrite_3.h"
#include <Vector2.h>
#include <Vector4.h>
#include <format>
#include <string>

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

class TextWriteManager;

class TextWrite {
public:
	~TextWrite();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="name">インスタンスの名前(被るのはダメ！)</param>
	void Initialize(const std::string& name);

	template <typename... Args>
	/// <summary>
	/// テキスト生成
	/// </summary>
	/// <param name="text">出力する文字</param>
	/// <param name="...args">{}に表示させる変数名</param>
	void WriteText(const std::wstring& text, Args&&... args) {
		//テキストを生成
		text_ = std::vformat(text, std::make_wformat_args(args...));
		//テキストを描画
		WriteOnManager();
	}
	//ImGuiを使ったデバッグ
	void DebugWithImGui();


public://セッター
	/// <summary>
	/// テキストのパラメーター設定(初期化時のみ)
	/// </summary>
	/// <param name="position">スクリーン座標</param>
	/// <param name="font">フォント名　(例 : Font::Meiryo)</param>
	/// <param name="size">サイズ　(平均は32.0fくらい)</param>
	/// <param name="color">色　(Vector4型)</param>
	void SetParam(const Vector2& position, const Font& font, float size, const Vector4& color);

	void SetPosition(const Vector2& position) { position_ = position; }
	void SetWidth(float width) { width_ = width; }
	void SetHeight(float height) { height_ = height; }
	void SetFont(const Font& font);
	void SetFontStyle(const FontStyle& fontStyle);
	void SetSize(float size);
	void SetColor(const Vector4& color);

	/// <summary>
	/// アウトラインのパラメーター設定
	/// </summary>
	/// <param name="color">色　(Vector4型)</param>
	/// <param name="strokeWidth">アウトラインの太さ　(平均は5.0fくらい)</param>
	/// <param name="slideRate">アウトラインのずらし量</param>
	/// <param name="isDisplay">表示させるか</param>
	void SetEdgeParam(const Vector4& color, float strokeWidth, const Vector2& slideRate, bool isDisplay);
	void SetEdgeColor(const Vector4& color);
	void SetEdgeStrokeWidth(float width) { edgeStrokeWidth_ = width; }
	void SetEdgeSlideRate(const Vector2& slideRate) { edgeSlideRate_ = slideRate; }
	void SetIsEdgeDisplay(bool isDisplay) { isEdgeDisplay_ = isDisplay; }
	
public://ゲッター
	const std::string& GetName() { return name_; }
	const std::wstring& GetText() { return text_; }
	const Vector2& GetPosition() { return position_; }
	float GetWidth() { return width_; }
	float GetHeight() { return height_; }
	const Font& GetFont() { return font_; }
	const std::wstring& GetFontName() { return fontName_; }
	const FontStyle& GetFontStyle() { return fontStyle_; }
	const std::string& GetFontFaceKey() { return fontFaceKey_; }
	float GetSize() { return size_; }
	const Vector4& GetColor() { return color_; }
	const std::string& GetEdgeName() { return edgeName_; }
	const Vector4& GetEdgeColor() { return edgeColor_; }
	float GetEdgeStrokeWidth() { return edgeStrokeWidth_; }
	const Vector2& GetEdgeSlideRate() { return edgeSlideRate_; }
	bool GetIsEdgeDisplay() { return isEdgeDisplay_; }

private:
	//フォントを入れたらフォント名を出す関数
	const std::wstring& ReturnFontName(const Font& font);
	//マネージャーに描画
	void WriteOnManager();


private:
	///============================
	/// テキスト
	///============================

	//識別名
	std::string name_;
	//テキスト
	std::wstring text_;
	//テキストの座標
	Vector2 position_;
	//テキストの幅
	float width_;
	//テキストの高さ
	float height_;
	//フォント(わかりやすい用)
	Font font_;
	//フォント名(正式)
	std::wstring fontName_;
	//フォントのスタイル
	FontStyle fontStyle_;
	//フォントフェイスのキー
	std::string fontFaceKey_;
	//テキストのサイズ
	float size_;
	//色
	Vector4 color_;

	///============================
	/// アウトライン
	///============================

	//識別名
	std::string edgeName_;
	//色
	Vector4 edgeColor_;
	//幅
	float edgeStrokeWidth_;
	//スライド量
	Vector2 edgeSlideRate_;
	//アウトライン表示フラグ
	bool isEdgeDisplay_;
};
