#include "TextTextureManager.h"
#include "GPUDescriptorManager.h"
#include "RTVManager.h"
#include "MainRender.h"
#include "ImGuiManager.h"
#include "Logger.h"
#include <algorithm>
#include <filesystem>
#include <format>
#include <cassert>

namespace fs = std::filesystem;

TextTextureManager* TextTextureManager::instance = nullptr;

TextTextureManager* TextTextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextTextureManager;
	}
	return instance;
}

void TextTextureManager::Initialize() {
	//DWriteFactoryの生成
	GenerateIDWriteFactory();
	//フォントファイルの生成
	GenerateFontFile();
	//グラフィックスパイプラインの生成
	GenerateGraphicsPipeline();
}

void TextTextureManager::Finalize() {
	delete instance;
	instance = nullptr;
}

void TextTextureManager::DebugWithImGui(Handle _handle) {
#ifdef _DEBUG
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	ImGui::Begin(std::format("テキストテクスチャ{}", _handle.id).c_str());
	//テキストの編集
	if (ImGui::CollapsingHeader("テキストの編集")) {
		//フォントの編集
		{
			const char* fontKind[] = {
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
			static int currentFontIndex = (int)GetTextFont(_handle);
			if (ImGui::Combo("フォントを選択", &currentFontIndex, fontKind, IM_ARRAYSIZE(fontKind))) {
				EditTextFont(_handle, (Font)currentFontIndex);
			}
		}
		//フォントスタイルの編集
		{
			const char* fontStyleKind[] = {
				"Normal",
				"Oblique",
				"Italic"
			};
			static int currentFontStyleIndex = (int)GetTextFontStyle(_handle);
			if (ImGui::Combo("フォントスタイルを選択", &currentFontStyleIndex, fontStyleKind, IM_ARRAYSIZE(fontStyleKind))) {
				EditTextFont(_handle, (Font)currentFontStyleIndex);
			}
		}
		//サイズの編集
		{
			static float currentSize = GetTextSize(_handle);
			ImGui::DragFloat("サイズを変更", &currentSize, 0.1f, 5.0f);
			EditTextSize(_handle, currentSize);
		}
		//カラーの編集
		{
			static Vector4 currentColor = GetTextColor(_handle);
			ImGui::ColorEdit4("カラーを変更", &currentColor.x);
			EditTextColor(_handle, currentColor);
		}
	}
	//アウトラインの編集
	if (ImGui::CollapsingHeader("アウトラインの編集")) {
		//表示・非表示の編集
		{
			static bool currentIsDisplay = GetIsEdgeDisplay(_handle);
			ImGui::Checkbox("表示・非表示設定", &currentIsDisplay);
			EditIsEdgeDisplay(_handle, currentIsDisplay);
		}
		//幅の編集
		{
			static float currentWidth = GetEdgeWidth(_handle);
			ImGui::DragFloat("幅を変更", &currentWidth, 0.1f, 0.0f, kMaxEdgeWidth_);
			EditEdgeWidth(_handle, currentWidth);
		}
		//スライド量の編集
		{
			static Vector2 currentSlideRate = GetEdgeSlideRate(_handle);
			ImGui::DragFloat2("スライド量を変更", &currentSlideRate.x, 0.1f, -5.0f, 5.0f);
			EditEdgeSlideRate(_handle, currentSlideRate);
		}
		//カラーの編集
		{
			static Vector4 currentColor = GetEdgeColor(_handle);
			ImGui::ColorEdit4("エッジカラーを変更", &currentColor.x);
			EditEdgeColor(_handle, currentColor);
		}
	}
	ImGui::End();
#endif // _DEBUG
}

Handle TextTextureManager::LoadTextTexture(const TextParam& _textParam) {
	//返却用のハンドルを生成
	Handle handle;
	//もしもfreeIndicesに値が入っていれば、それを使う
	if (!freeIndices.empty()) {
		handle.Create(freeIndices.back());
		freeIndices.pop_back();
	}
	//なければ最新のindexからもってくる
	else {
		handle.Create(useIndex);
		useIndex++;
	}
	//参照カウンタに登録
	referenceCounter.push_back(handle.Share());

	//テキストテクスチャアイテムを生成
	TextTextureItem textTextureItem;
	textTextureItem = CreateTextTextureItem(_textParam);

	//テキストテクスチャアイテムをコンテナに登録
	textTextureMap[handle.id] = textTextureItem;

	//return
	return handle.Share();
}

Handle TextTextureManager::LoadTextTexture(uint32_t _id) {
	//返却用のハンドルを生成
	Handle handle;
	//参照カウンタを全て回し、該当のidが見つかったらhandleにshareする
	for (Handle ref : referenceCounter) {
		if (ref.id == _id) {
			handle = ref.Share();
			break;
		}
	}
	//もしも見つからなければ、エラーを返す
	if (!handle.ref) {
		assert(0 && "指定されたIDのテクスチャは見つかりませんでした。");
	}
	return handle;
}

void TextTextureManager::EditTextParam(Handle _handle, const TextParam& _textParam) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//該当のコンテナに値を代入する
	textTextureMap[_handle.id].textResource.param = _textParam;
	*textTextureMap[_handle.id].textResource.color = _textParam.color;
	//ブラシとテキストフォーマットの更新
	textTextureMap[_handle.id].solidColorBrush = CreateSolidColorBrush(_textParam.color);
	textTextureMap[_handle.id].textFormat = CreateTextFormat(_textParam.font, _textParam.fontStyle, _textParam.size);
}

void TextTextureManager::EditEdgeParam(Handle _handle, const EdgeParam& _edgeParam) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//該当のコンテナに値を代入する
	EdgeParam param = _edgeParam;
	*textTextureMap[_handle.id].edgeResource.param = param;
}

void TextTextureManager::EditTextFont(Handle _handle, const Font& _font) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//フォントの編集
	textTextureMap[_handle.id].textResource.param.font = _font;
	//フォントを反映
	textTextureMap[_handle.id].textFormat = CreateTextFormat(textTextureMap[_handle.id].textResource.param.font, textTextureMap[_handle.id].textResource.param.fontStyle, textTextureMap[_handle.id].textResource.param.size);
}

void TextTextureManager::EditTextFontStyle(Handle _handle, const FontStyle& _fontStyle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//フォントスタイルの編集
	textTextureMap[_handle.id].textResource.param.fontStyle = _fontStyle;
	//フォントスタイルを反映
	textTextureMap[_handle.id].textFormat = CreateTextFormat(textTextureMap[_handle.id].textResource.param.font, textTextureMap[_handle.id].textResource.param.fontStyle, textTextureMap[_handle.id].textResource.param.size);
}

void TextTextureManager::EditTextSize(Handle _handle, const float _size) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//サイズの編集
	textTextureMap[_handle.id].textResource.param.size = _size;
	//サイズを反映
	textTextureMap[_handle.id].textFormat = CreateTextFormat(textTextureMap[_handle.id].textResource.param.font, textTextureMap[_handle.id].textResource.param.fontStyle, textTextureMap[_handle.id].textResource.param.size);
}

void TextTextureManager::EditTextColor(Handle _handle, const Vector4& _color) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//カラーの編集
	textTextureMap[_handle.id].textResource.param.color = _color;
	*textTextureMap[_handle.id].textResource.color = _color;
	//カラーを反映
	textTextureMap[_handle.id].solidColorBrush = CreateSolidColorBrush(textTextureMap[_handle.id].textResource.param.color);
}

void TextTextureManager::EditIsEdgeDisplay(Handle _handle, const bool _isDisplay) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//ディスプレイの編集
	textTextureMap[_handle.id].edgeResource.param->isEdgeDisplay = _isDisplay;
}

void TextTextureManager::EditEdgeWidth(Handle _handle, const float _width) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//最大幅でクランプ
	float width = std::clamp(_width, 0.0f, kMaxEdgeWidth_);
	//幅の編集
	textTextureMap[_handle.id].edgeResource.param->width = width;
}

void TextTextureManager::EditEdgeSlideRate(Handle _handle, const Vector2& _slideRate) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//スライド量の編集
	textTextureMap[_handle.id].edgeResource.param->slideRate = _slideRate;
}

void TextTextureManager::EditEdgeColor(Handle _handle, const Vector4& _color) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//カラーの編集
	textTextureMap[_handle.id].edgeResource.param->color = _color;
}

const UINT TextTextureManager::GetTextureWidth(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].width;
}

const UINT TextTextureManager::GetTextureHeight(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].height;
}

const std::wstring& TextTextureManager::GetTextString(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].textResource.param.text;
}

const Font& TextTextureManager::GetTextFont(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].textResource.param.font;
}

const FontStyle& TextTextureManager::GetTextFontStyle(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].textResource.param.fontStyle;
}

const float TextTextureManager::GetTextSize(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].textResource.param.size;
}

const Vector4& TextTextureManager::GetTextColor(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].textResource.param.color;
}

const bool TextTextureManager::GetIsEdgeDisplay(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].edgeResource.param->isEdgeDisplay;
}

const float TextTextureManager::GetEdgeWidth(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].edgeResource.param->width;
}

const Vector2& TextTextureManager::GetEdgeSlideRate(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].edgeResource.param->slideRate;
}

const Vector4& TextTextureManager::GetEdgeColor(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	return textTextureMap[_handle.id].edgeResource.param->color;
}

uint32_t TextTextureManager::GetSrvIndex(Handle _handle) {
	//使用可能なハンドルかチェック
	CheckHandle(_handle);

	//該当コンテナからsrvIndexを出力
	return textTextureMap[_handle.id].srvIndex;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextTextureManager::GetSrvHandleGPU(Handle _handle) {

	//該当コンテナからsrvIndexをもとにSRVDescriptorHandleを出力
	return GPUDescriptorManager::GetInstance()->GetGPUDescriptorHandle(textTextureMap[_handle.id].srvIndex);
}

void TextTextureManager::CheckHandle(Handle _handle) {
	//参照カウンタから使用可能なハンドルかチェック
	bool isValid = std::any_of(referenceCounter.begin(), referenceCounter.end(),
		[&_handle](const Handle& handle) {
			return _handle.ref == handle.ref;
		});
	if (!isValid) {
		assert(0 && "無効なハンドルです");
		return;
	}
}

TextTextureManager::TextTextureItem TextTextureManager::CreateTextTextureItem(const TextParam& _textParam) {
	TextTextureItem textTextureItem;
	//ローカル変数
	HRESULT hr;

	//縦横の初期化
	{
		textTextureItem.width = UINT_MAX;
		textTextureItem.height = UINT_MAX;
	}
	//ブラシの作成
	{
		textTextureItem.solidColorBrush = CreateSolidColorBrush(_textParam.color);
	}
	//テキストフォーマットの作成
	{
		textTextureItem.textFormat = CreateTextFormat(_textParam.font, _textParam.fontStyle, _textParam.size);
	}
	//テキストリソースの作成
	{
		textTextureItem.textResource.resource = dxcommon->CreateBufferResource(sizeof(Vector4));
		textTextureItem.textResource.resource->Map(0, nullptr, reinterpret_cast<void**>(&textTextureItem.textResource.color));
		*textTextureItem.textResource.color = _textParam.color;

		textTextureItem.textResource.param = _textParam;
	}
	//アウトラインリソースの作成(初期化兼非表示設定)
	{
		EdgeResource edgeResource;
		edgeResource.resource = dxcommon->CreateBufferResource(sizeof(EdgeParam));
		edgeResource.resource->Map(0, nullptr, reinterpret_cast<void**>(&edgeResource.param));
		edgeResource.param->isEdgeDisplay = 0;

		textTextureItem.edgeResource = edgeResource;
	}

	return textTextureItem;
}

ComPtr<ID2D1SolidColorBrush> TextTextureManager::CreateSolidColorBrush(const Vector4& color) {
	HRESULT hr;
	//色と透明度を分離
	D2D1::ColorF rgb(color.x, color.y, color.z);
	FLOAT alpha = static_cast<FLOAT>(color.w);
	//ブラシを作って登録(すでに作っていたら編集)
	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	hr = d2drender->GetD2DDeviceContext()->CreateSolidColorBrush(rgb, &brush);
	assert(SUCCEEDED(hr));
	brush->SetOpacity(alpha);
	return brush;
}

ComPtr<IDWriteTextFormat> TextTextureManager::CreateTextFormat(const Font& _font, const FontStyle& _fontStyle, const float fontSize) noexcept {
	HRESULT hr;
	//フォント情報からフォント名を取得
	std::wstring fontName;
	switch (_font) {
	case Font::Meiryo:
		fontName = L"Meiryo";
		break;
	case Font::YuGothic:
		fontName = L"Yu Gothic";
		break;
	case Font::YuMincho:
		fontName = L"Yu Mincho";
		break;
	case Font::UDDegitalN_B:
		fontName = L"UD Digi Kyokasho N-B";
		break;
	case Font::UDDegitalN_R:
		fontName = L"UD Digi Kyokasho N-R";
		break;
	case Font::UDDegitalNK_B:
		fontName = L"UD Digi Kyokasho NK-B";
		break;
	case Font::UDDegitalNK_R:
		fontName = L"UD Digi Kyokasho NK-R";
		break;
	case Font::UDDegitalNP_B:
		fontName = L"UD Digi Kyokasho NP-B";
		break;
	case Font::UDDegitalNP_R:
		fontName = L"UD Digi Kyokasho NP-R";
		break;
	case Font::OnionScript:
		fontName = L"Tamanegi Kaisho Geki FreeVer 7";
		break;
	default:
		assert(0 && "フォント名が不正です。");
		break;
	}

	ComPtr<IDWriteTextFormat> textFormat = nullptr;
	DWRITE_FONT_STYLE style = static_cast<DWRITE_FONT_STYLE>(_fontStyle);
	hr = directWriteFactory->CreateTextFormat(
		fontName.c_str(),
		dwriteFontCollection.Get(),
		DWRITE_FONT_WEIGHT_NORMAL,
		style,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"ja-jp",
		&textFormat
	);
	assert(SUCCEEDED(hr));

	//中央に揃える設定(今後はここもカスタムできるようにしたい)
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	return textFormat;
}

void TextTextureManager::TransitionState(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = pResource;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;

	ttrender->GetCommandList()->ResourceBarrier(1, &barrier);
}

void TextTextureManager::GenerateIDWriteFactory() {
	HRESULT hr;
	//IDWriteFactoryの生成
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &directWriteFactory);
	assert(SUCCEEDED(hr));
}

void TextTextureManager::GenerateFontFile() {
	HRESULT hr;
	// IDWriteFontSetBuilder2 の生成
	ComPtr<IDWriteFontSetBuilder2> dwriteFontSetBuilder = nullptr;
	hr = directWriteFactory->CreateFontSetBuilder(&dwriteFontSetBuilder);
	assert(SUCCEEDED(hr));
	// フォントファイルとフォントフェイスを作る
	std::vector<ComPtr<IDWriteFontFile>> fontFiles;
	std::wstring fontDirectory = L"Resources/fonts"; // フォントフォルダのパス

	// fontsフォルダ内の .ttf 及び .ttc ファイルを探索
	for (const auto& entry : fs::directory_iterator(fontDirectory)) {
		if (entry.is_regular_file() && (entry.path().extension() == L".ttf" || entry.path().extension() == L".ttc")) {
			// IDWriteFontFile の生成
			ComPtr<IDWriteFontFile> dwriteFontFile;
			hr = directWriteFactory->CreateFontFileReference(entry.path().c_str(), nullptr, &dwriteFontFile);
			if (FAILED(hr)) continue;
			// vectorに保存
			fontFiles.push_back(dwriteFontFile);
			// フォントセットビルダーに追加
			hr = dwriteFontSetBuilder->AddFontFile(dwriteFontFile.Get());
			assert(SUCCEEDED(hr));

			//フォントファイルの種類を取得
			BOOL isSupported;
			DWRITE_FONT_FILE_TYPE fileType;
			DWRITE_FONT_FACE_TYPE faceType;
			UINT32 numFaces;
			hr = dwriteFontFile->Analyze(&isSupported, &fileType, &faceType, &numFaces);
			if (FAILED(hr) || !isSupported) continue;

			// IDWriteFontFace の作成 (ttcの場合はフォントごとに作る)
			for (UINT32 i = 0; i < numFaces; i++) {
				//IDWriteFontReferenceの作成
				ComPtr<IDWriteFontFaceReference> dwriteFontFaceRef;
				hr = directWriteFactory->CreateFontFaceReference(
					dwriteFontFile.Get(), i, DWRITE_FONT_SIMULATIONS_NONE, &dwriteFontFaceRef
				);
				if (FAILED(hr)) continue;
				//IDWriteFontFaceの作成
				ComPtr<IDWriteFontFace3> dwriteFontFace;
				hr = dwriteFontFaceRef->CreateFontFace(dwriteFontFace.ReleaseAndGetAddressOf());
				if (FAILED(hr)) continue;
				//フォントファミリー名を取得
				ComPtr<IDWriteLocalizedStrings> fontNames;
				hr = dwriteFontFace->GetFamilyNames(&fontNames);
				if (FAILED(hr)) continue;
				// 最初のフォント名を取得
				UINT32 length = 0;
				hr = fontNames->GetStringLength(0, &length);
				if (FAILED(hr)) continue;
				std::vector<wchar_t> nameBuffer(length + 1);
				hr = fontNames->GetString(0, nameBuffer.data(), length + 1);
				if (FAILED(hr)) continue;
				std::wstring fontName = nameBuffer.data();

				// フォントのスタイルを取得
				DWRITE_FONT_STYLE fontStyle = dwriteFontFace->GetStyle();
				FontStyle style = static_cast<FontStyle>(fontStyle);
				std::string fontKey = GenerateFontKey(fontName, style);

				// unordered_map に格納
				fontFaceMap[fontKey] = dwriteFontFace;
			}
		}
	}

	// IDWriteFontSet の生成
	ComPtr<IDWriteFontSet> dwriteFontSet = nullptr;
	hr = dwriteFontSetBuilder->CreateFontSet(&dwriteFontSet);
	assert(SUCCEEDED(hr));

	// フォントコレクションの生成
	hr = directWriteFactory->CreateFontCollectionFromFontSet(dwriteFontSet.Get(), &dwriteFontCollection);
	assert(SUCCEEDED(hr));
}

std::string TextTextureManager::GenerateFontKey(const std::wstring& fontName, const FontStyle& style) {
	std::string key(fontName.begin(), fontName.end()); // wstring → string 変換

	switch (style) {
	case FontStyle::Normal:  key += "_Normal"; break;
	case FontStyle::Oblique: key += "_Oblique"; break;
	case FontStyle::Italic:  key += "_Italic"; break;
	}
	return key;
}

void TextTextureManager::GenerateGraphicsPipeline() {
	HRESULT hr;
	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	//テクスチャの設定
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;		//Tableを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;					//PixelShaderで使う
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;				//Tableの中身の配列を指定
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);	//Rangeのサイズ
	//テキストの情報の設定
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PSで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;						//Register番号は0
	//アウトラインの情報の設定
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PSで使う
	rootParameters[2].Descriptor.ShaderRegister = 1;						//Register番号は1

	//Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//Signatureに反映
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	descriptionRootSignature.pParameters = rootParameters;//ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);//配列の長さ

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3D10Blob> signatireBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatireBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリをもとに生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatireBlob->GetBufferPointer(),
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	//InputLayoutは利用しない
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = nullptr;
	inputLayoutDesc.NumElements = 0;

	//ブレンドの設定
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE; // アルファ値を考慮する
	blendDesc.IndependentBlendEnable = FALSE;
	D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc{};
	rtBlendDesc.BlendEnable = TRUE; // ブレンド有効
	rtBlendDesc.LogicOpEnable = FALSE;
	rtBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rtBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0] = rtBlendDesc;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面を表示する(モデルとは無関係)
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/TextTexture.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DirectXCommon::GetInstance()->CompileShader(L"Resources/shaders/TextTexture.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を無効化する
	depthStencilDesc.DepthEnable = false;

	//グラフィックスパイプラインステートに設定を反映
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	//書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジのタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	//実際に生成
	hr = DirectXCommon::GetInstance()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));

}

void TextTextureManager::CheckAllReference() {
	//参照カウンタリストを全て回してHandleの参照カウントを数える
	for (auto it = referenceCounter.begin(); it != referenceCounter.end();) {
		const Handle& handle = *it;

		//他クラスに参照されていない(使われていない)ので削除
		if (handle.ref.use_count() <= 1) {
			//RTVインデックスを解放
			RTVManager::GetInstance()->Free(textTextureMap[handle.id].rtvIndex);
			//SRVインデックスを解放
			GPUDescriptorManager::GetInstance()->Free(textTextureMap[handle.id].srvIndex);
			//コンテナの要素を削除
			textTextureMap.erase(handle.id);
			//空きインデックスにidを登録
			freeIndices.push_back(handle.id);
			//参照カウンタから削除（erase するとイテレータ無効になるので再代入）
			it = referenceCounter.erase(it);
		}
		//参照確認したので次へ
		else {
			++it;
		}
	}
}

void TextTextureManager::ArrangeTextureSize(uint32_t _id) {
	HRESULT hr;
	const auto rtvManager = RTVManager::GetInstance();
	const auto srvManager = GPUDescriptorManager::GetInstance();

	//テキストのレイアウトを生成
	ComPtr<IDWriteTextLayout> textLayout;
	hr = directWriteFactory->CreateTextLayout(
		textTextureMap[_id].textResource.param.text.c_str(),							//描画テキスト
		static_cast<UINT32>(textTextureMap[_id].textResource.param.text.size()),	//文字数		
		textTextureMap[_id].textFormat.Get(),												//フォント
		FLT_MAX,																		//最大幅
		FLT_MAX,																		//最大高さ
		&textLayout
	);
	assert(SUCCEEDED(hr));

	//メトリクスを取得
	DWRITE_TEXT_METRICS metrics = {};
	hr = textLayout->GetMetrics(&metrics);
	assert(SUCCEEDED(hr));

	//テクスチャサイズの決定
	UINT textureWidth = static_cast<UINT>(ceil(metrics.width + (kMaxEdgeWidth_ * 2.0f)));
	UINT textureHeight = static_cast<UINT>(ceil(metrics.height + (kMaxEdgeWidth_ * 2.0f)));

	//もしテクスチャサイズがずれていた場合、リソースを作り直す。(軽量化のため初期化も兼ねる)
	if (textureWidth != textTextureMap[_id].width || textureHeight != textTextureMap[_id].height) {
		//D3Dレンダーターゲット用ID3D12Resourceの作成
		{
			//リセット
			textTextureMap[_id].resource.Reset();
			if (textTextureMap[_id].rtvIndex != 0) {
				rtvManager->Free(textTextureMap[_id].rtvIndex);
			}
			if (textTextureMap[_id].srvIndex != 0) {
				srvManager->Free(textTextureMap[_id].srvIndex);
			}
			//リソースの作成
			textTextureMap[_id].resource = dxcommon->CreateRenderTextureResource(textureWidth, textureHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			//RTVの作成
			textTextureMap[_id].rtvIndex = rtvManager->Allocate();
			rtvManager->CreateRTVDescriptor(textTextureMap[_id].rtvIndex, textTextureMap[_id].resource.Get());
			//SRVの作成
			textTextureMap[_id].srvIndex = srvManager->Allocate();
			srvManager->CreateSRVforRenderTexture(textTextureMap[_id].srvIndex, textTextureMap[_id].resource.Get());
		}
		//テクスチャコピー用ID3D12Resourceの作成
		{
			//リセット
			textTextureMap[_id].copyResource.Reset();
			//リソースの作成
			textTextureMap[_id].copyResource = dxcommon->CreateRenderTextureResource(textureWidth, textureHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			TransitionState(textTextureMap[_id].copyResource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
		//D2Dレンダーターゲット用BitMapと中継用のID3D11Resourceの作成
		{
			//リセット
			textTextureMap[_id].wrappedResource.Reset();
			textTextureMap[_id].d2dRenderTarget.Reset();
			//DirectWriteの描画先の生成
			D3D11_RESOURCE_FLAGS resourceFlags = { D3D11_BIND_RENDER_TARGET };
			const UINT dpi = GetDpiForWindow(winapp->GetHwnd());
			D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), static_cast<float>(dpi), static_cast<float>(dpi));
			//D2Dで使える用のリソースを生成
			ComPtr<ID3D11Resource> wrappedTextureResource = nullptr;
			//ID3D11Resourceの生成
			hr = d2drender->GetD3D11On12Device()->CreateWrappedResource(textTextureMap[_id].resource.Get(), &resourceFlags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, IID_PPV_ARGS(&wrappedTextureResource));
			assert(SUCCEEDED(hr));
			//IDXGISurfaceの生成
			ComPtr<IDXGISurface> dxgiSurface = nullptr;
			hr = wrappedTextureResource.As(&dxgiSurface);
			assert(SUCCEEDED(hr));
			//ID2D1Bitmap1の生成
			ComPtr<ID2D1Bitmap1> d2dRenderTarget = nullptr;
			hr = d2drender->GetD2DDeviceContext()->CreateBitmapFromDxgiSurface(dxgiSurface.Get(), &bitmapProperties, &d2dRenderTarget);
			assert(SUCCEEDED(hr));

			//作成した変数をメンバ変数に格納
			textTextureMap[_id].wrappedResource = wrappedTextureResource;
			textTextureMap[_id].d2dRenderTarget = d2dRenderTarget;
		}
	}

	//設定内容を反映
	textTextureMap[_id].width = textureWidth;
	textTextureMap[_id].height = textureHeight;
}

void TextTextureManager::WriteTextOnD2D() {
	//参照カウントをチェック(未参照コンテナを削除する)
	CheckAllReference();

	//各コンテナの処理
	for (auto& [id, item] : textTextureMap) {
		//テクスチャの縦幅、横幅を決めて反映する処理
		ArrangeTextureSize(id);
		//描画範囲の決定
		D2D1_RECT_F rect;	//描画範囲
		rect = {
			0.0f,
			0.0f,
			(float)item.width,
			(float)item.height
		};
		
		//リソースの設定
		d2drender->GetD3D11On12Device()->AcquireWrappedResources(item.wrappedResource.GetAddressOf(), 1);
		//描画ターゲットの設定
		d2drender->GetD2DDeviceContext()->SetTarget(item.d2dRenderTarget.Get());
		//描画前処理
		d2drender->GetD2DDeviceContext()->BeginDraw();
		//テキストテクスチャ描画処理
		d2drender->GetD2DDeviceContext()->SetTransform(
			D2D1::Matrix3x2F::Identity()
		);
		//描画対象のクリア
		d2drender->GetD2DDeviceContext()->Clear({ 0.0f,0.0f,0.0f,0.0f });
		d2drender->GetD2DDeviceContext()->DrawTextW(
			item.textResource.param.text.c_str(),
			static_cast<UINT32>(item.textResource.param.text.length()),
			item.textFormat.Get(),
			&rect,
			item.solidColorBrush.Get()
		);
		//描画後処理
		d2drender->GetD2DDeviceContext()->EndDraw();
		//リソースを取り外す
		d2drender->GetD3D11On12Device()->ReleaseWrappedResources(item.wrappedResource.GetAddressOf(), 1);
	}
	//描画内容の確定（ExecuteCommandListみたいなやつ→処理がまとまったら1回呼び出せば十分）
	d2drender->GetD3D11On12DeviceContext()->Flush();

}

void TextTextureManager::DrawDecorationOnD3D12() {
	//ルートシグネチャをセットするコマンド
	ttrender->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//グラフィックスパイプラインステートをセットするコマンド
	ttrender->GetCommandList()->SetPipelineState(graphicsPipelineState_.Get());
	//プリミティブトポロジーをセットするコマンド
	ttrender->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//各コンテナの処理
	for (auto& [id, item] : textTextureMap) {
		//コピーするためにコピー元をSourceコピー先をDestにする
		TransitionState(item.resource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE);
		TransitionState(item.copyResource.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		//現在のテクスチャ(D2Dで描画した)のピクセル情報をコピーリソースに写す
		ttrender->GetCommandList()->CopyResource(item.copyResource.Get(), item.resource.Get());
		//コピー済みのリソースをPixel_Shader_Resourceにする
		TransitionState(item.copyResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		//コピーリソースにSRVの設定
		item.srvCopyIndex = GPUDescriptorManager::GetInstance()->Allocate();
		GPUDescriptorManager::GetInstance()->CreateSRVforRenderTexture(item.srvCopyIndex, item.copyResource.Get());

		//リソースのステートをRender_Targetに変更
		TransitionState(item.resource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

		//描画先のRTVの設定をする
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RTVManager::GetInstance()->GetCPUDescriptorHandle(item.rtvIndex);
		ttrender->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
		//クリアバリューの色で画面全体をクリアする
		float clearColor[] = {
			0.0f,0.0f,0.0f,0.0f
		};
		ttrender->GetCommandList()->ClearRenderTargetView(RTVManager::GetInstance()->GetCPUDescriptorHandle(item.rtvIndex), clearColor, 0, nullptr);

		//ビューポートの送信
		TextTextureRender::GetInstance()->SettingViewPort(item.width, item.height);
		//シザー矩形の送信
		TextTextureRender::GetInstance()->SettingScissorRect(item.width, item.height);

		//テクスチャの送信
		ttrender->GetCommandList()->SetGraphicsRootDescriptorTable(0, GPUDescriptorManager::GetInstance()->GetGPUDescriptorHandle(item.srvCopyIndex));
		//テキスト情報の送信
		ttrender->GetCommandList()->SetGraphicsRootConstantBufferView(1, item.textResource.resource->GetGPUVirtualAddress());
		//アウトライン情報の送信
		ttrender->GetCommandList()->SetGraphicsRootConstantBufferView(2, item.edgeResource.resource->GetGPUVirtualAddress());

		//ドローコール
		ttrender->GetCommandList()->DrawInstanced(6, 1, 0, 0);

		//リソースのステートをPixel_Shader_Rsourceに変更
		TransitionState(item.resource.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		//コピーリソースのSRVインデックスを破棄
		GPUDescriptorManager::GetInstance()->Free(item.srvCopyIndex);
	}
}

void TextTextureManager::ReadyNextResourceState() {
	const auto mainrender = MainRender::GetInstance();
	//リソースはMainRenderで使うため、MainRenderのcommandListで最後の遷移を行う
	for (auto& [id, item] : textTextureMap) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = item.resource.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		mainrender->GetCommandList()->ResourceBarrier(1, &barrier);
	}
}
