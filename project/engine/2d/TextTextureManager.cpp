#include "TextTextureManager.h"
#include "GPUDescriptorManager.h"
#include "RTVManager.h"
#include "MainRender.h"
#include <filesystem>
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
	CreateIDWriteFactory();
	//フォントファイルの生成
	CreateFontFile();
}

void TextTextureManager::Finalize() {
	delete instance;
	instance = nullptr;
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
	//参照カウンタから使用可能なハンドルかチェック
	bool isValid = std::any_of(referenceCounter.begin(), referenceCounter.end(),
		[&_handle](const Handle& handle) {
			return _handle.ref == handle.ref;
		});
	if (!isValid) {
		assert(0 && "無効なハンドルです");
		return;
	}
	//該当のコンテナに値を代入する
	textTextureMap[_handle.id].textParam = _textParam;
	//ブラシとテキストフォーマットの更新
	textTextureMap[_handle.id].solidColorBrush = CreateSolidColorBrush(_textParam.color);
	textTextureMap[_handle.id].textFormat = CreateTextFormat(_textParam.font, _textParam.fontStyle, _textParam.size);
}

void TextTextureManager::EditEdgeParam(Handle _handle, const EdgeParam& _edgeParam) {
	//参照カウンタから使用可能なハンドルかチェック
	bool isValid = std::any_of(referenceCounter.begin(), referenceCounter.end(),
		[&_handle](const Handle& handle) {
			return _handle.ref == handle.ref;
		});
	if (!isValid) {
		assert(0 && "無効なハンドルです");
		return;
	}
	//該当のコンテナに値を代入する
	textTextureMap[_handle.id].edgeParam = _edgeParam;
}

uint32_t TextTextureManager::GetSrvIndex(Handle _handle) {
	//参照カウンタから使用可能なハンドルかチェック
	bool isValid = std::any_of(referenceCounter.begin(), referenceCounter.end(),
		[&_handle](const Handle& handle) {
			return _handle.ref == handle.ref;
		});
	if (!isValid) {
		assert(0 && "無効なハンドルです");
	}
	//該当コンテナからsrvIndexを出力
	return textTextureMap[_handle.id].srvIndex;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextTextureManager::GetSrvHandleGPU(Handle _handle) {
	//参照カウンタから使用可能なハンドルかチェック
	bool isValid = std::any_of(referenceCounter.begin(), referenceCounter.end(),
		[&_handle](const Handle& handle) {
			return _handle.ref == handle.ref;
		});
	if (!isValid) {
		assert(0 && "無効なハンドルです");
	}
	//該当コンテナからsrvIndexをもとにSRVDescriptorHandleを出力
	return GPUDescriptorManager::GetInstance()->GetGPUDescriptorHandle(textTextureMap[_handle.id].srvIndex);
}

TextTextureManager::TextTextureItem TextTextureManager::CreateTextTextureItem(const TextParam& _textParam) {
	TextTextureItem textTextureItem;
	//ローカル変数
	HRESULT hr;
	const auto gpuDescriptorManager = GPUDescriptorManager::GetInstance();
	const auto rtvManager = RTVManager::GetInstance();

	////////////////////D3D12用リソースの作成////////////////////

	//リソースの作成
	textTextureItem.resource = dxcommon->CreateRenderTextureResource(WinApp::kClientWidth, WinApp::kClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, Vector4(0, 0, 0, 0), true);
	//rtvIndexの割り当て
	textTextureItem.rtvIndex = rtvManager->Allocate();
	//RTVを作成
	rtvManager->CreateRTVDescriptor(textTextureItem.rtvIndex, textTextureItem.resource.Get());
	//srvIndexの割り当て
	textTextureItem.srvIndex = gpuDescriptorManager->Allocate();
	//SRVを作成
	gpuDescriptorManager->CreateSRVforRenderTexture(textTextureItem.srvIndex, textTextureItem.resource.Get());

	////////////////////D3D11用リソースの作成////////////////////

	//DirectWriteの描画先の生成
	D3D11_RESOURCE_FLAGS resourceFlags = { D3D11_BIND_RENDER_TARGET };
	const UINT dpi = GetDpiForWindow(winapp->GetHwnd());
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), static_cast<float>(dpi), static_cast<float>(dpi));

	//D2Dで使える用のリソースを生成
	ComPtr<ID3D11Resource> wrappedTextureResource = nullptr;
	//ID3D11Resourceの生成
	hr = d2drender->GetD3D11On12Device()->CreateWrappedResource(textTextureItem.resource.Get(), &resourceFlags, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, IID_PPV_ARGS(&wrappedTextureResource));
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
	textTextureItem.wrappedResource = wrappedTextureResource;

	////////////////////D2D用レンダーターゲットの作成////////////////////

	textTextureItem.d2dRenderTarget = d2dRenderTarget;

	////////////////////ブラシの作成////////////////////

	textTextureItem.solidColorBrush = CreateSolidColorBrush(_textParam.color);

	////////////////////テキストフォーマットの作成////////////////////

	textTextureItem.textFormat = CreateTextFormat(_textParam.font, _textParam.fontStyle, _textParam.size);

	////////////////////テキストパラメータのセット////////////////////

	textTextureItem.textParam = _textParam;

	////////////////////アウトラインパラメータのセット(初期化と非表示設定)////////////////////

	EdgeParam edgeParam = {};
	edgeParam.isEdgeDisplay = 0;
	textTextureItem.edgeParam = edgeParam;

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

void TextTextureManager::CreateIDWriteFactory() {
	HRESULT hr;
	//IDWriteFactoryの生成
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &directWriteFactory);
	assert(SUCCEEDED(hr));
}

void TextTextureManager::CreateFontFile() {
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

void TextTextureManager::WriteTextOnD2D() {
	//参照カウントをチェック(未参照コンテナを削除する)
	CheckAllReference();

	//全コンテナ共通処理
	D2D1_RECT_F rect;	//描画範囲
	rect = {
		0.0f,
		0.0f,
		WinApp::kClientWidth,
		WinApp::kClientHeight
	};

	//各コンテナの処理
	for (auto& [id, item] : textTextureMap) {

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
		d2drender->GetD2DDeviceContext()->DrawTextW(
			item.textParam.text.c_str(),
			static_cast<UINT32>(item.textParam.text.length()),
			item.textFormat.Get(),
			&rect,
			item.solidColorBrush.Get()
		);
		//描画後処理
		d2drender->GetD2DDeviceContext()->EndDraw();
		//リソースを取り外す
		d2drender->GetD3D11On12Device()->ReleaseWrappedResources(item.wrappedResource.GetAddressOf(), 1);
	}

	//描画内容の確定（ExecuteCommandListみたいなやつ→1回呼び出せば十分）
	d2drender->GetD3D11On12DeviceContext()->Flush();
}

void TextTextureManager::DrawDecorationOnD3D12() {
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
