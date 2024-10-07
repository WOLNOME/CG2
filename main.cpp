#define _USE_MATH_DEFINES
#include <windows.h>
#include <fstream>
#include <sstream>
#include <xaudio2.h>
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Function.h"
#include "DirectXCommon.h"
#include "WinApp.h"
#include "Input.h"
#include "D3DResourceLeakChecker.h"
#include "Logger.h"

#pragma comment(lib,"xaudio2.lib")

//定数
const int kTriangleVertexNum = 3;
const int kTriangleNum = 2;

const uint32_t kSubdivision = 20;
float pi = (float)M_PI;

//モデルリソース作成用データ型
struct ModelResource {
	std::vector<ModelData> modelData;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResource;
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView;
	std::vector<VertexData*> vertexData;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResource;
	std::vector<Material*> materialData;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	TransformationMatrix* wvpData;
	Transform transform;
	std::vector<Transform> uvTransform;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResorce;
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU;
	std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU;
};

//チャンクヘッダ
struct ChunkHeader
{
	char id[4];
	int32_t size;
};

//RIFFヘッダチャンク
struct RiffHeader
{
	ChunkHeader chunk;
	char type[4];
};

//FMTチャンク
struct FormatChunk
{
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

//音声データ
struct SoundData
{
	//波形フォーマット
	WAVEFORMATEX wfex;
	//バッファの先頭アドレス
	BYTE* pBuffer;
	//バッファのサイズ
	unsigned int bufferSize;
};

//mtlファイルを読む関数
MaterialData LoadMaterialTemplateFile(const std::string& directryPath, const std::string& filename, const std::string& materialName) {
	//1.中で必要となる変数の宣言
	MaterialData materialData;
	std::string line;
	bool isLoad = false;
	//2.ファイルを開く
	std::ifstream file(directryPath + "/" + filename);
	assert(file.is_open());
	//3.実際にファイルを読み、MaterialDAtaを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;
		//identifierに応じた処理
		if (identifier == "newmtl") {
			std::string mtlNumber;
			s >> mtlNumber;
			//読み込みたいマテリアルの名前が一致してたら読み込み、そうじゃないなら読み込まない
			if (mtlNumber == materialName) {
				isLoad = true;
			}
			else {
				isLoad = false;
			}
		}
		if (isLoad) {
			if (identifier == "map_Kd") {
				std::string textureFilename;
				s >> textureFilename;
				//連結してファイルパスにする
				materialData.textureFilePath = directryPath + "/" + textureFilename;
			}
			else if (identifier == "Kd") {
				Vector4 color;
				s >> color.x >> color.y >> color.z;
				color.w = 1.0f;
				materialData.colorData = color;
			}
		}
	}
	//4.MaterialDataを返す
	return materialData;
}

//objファイル読む関数
std::vector<ModelData> LoadObjFIle(const std::string& directoryPath, const std::string& filename) {
	//1.中で必要となる変数の宣言
	std::vector<ModelData> modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;
	std::string line2;
	bool isGetO = false;
	bool isGetVt = false;
	size_t index = -1;
	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());
	//3.実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//←先頭の識別子を読む
		//identifierに応じた処理
		if (identifier == "o") {
			index++;
			modelData.resize(index + 1);
			isGetO = true;
		}
		else if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
			isGetVt = true;
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			//面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのstrは「位置/UV/法線」で格納されているので、分解してstrを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string str;
					std::getline(v, str, '/');
					if (str.size() == 0) {
						//データがない場合、数値0を仮に入れておく
						str = "0";
					}
					elementIndices[element] = std::stoi(str);
				}
				//要素へのstrから、実際の要素の値を取得して、頂点を構築する
				Vector4 position;
				Vector2 texcoord;
				Vector3 normal;
				if (isGetVt) {
					position = positions[elementIndices[0] - 1];
					texcoord = texcoords[elementIndices[1] - 1];
					normal = normals[elementIndices[2] - 1];
				}
				else {
					position = positions[elementIndices[0] - 1];
					texcoord = { 0,0 };//texcoord(0,0)を代入
					normal = normals[elementIndices[2] - 1];
				}
				triangle[faceVertex] = { position,texcoord,normal };

			}
			//頂点を逆順に登録することで、周り順を逆にする
			modelData.at(index).vertices.push_back(triangle[2]);
			modelData.at(index).vertices.push_back(triangle[1]);
			modelData.at(index).vertices.push_back(triangle[0]);
		}
		else if (identifier == "usemtl") {
			std::string materialName;
			s >> materialName;
			//例外処理(objファイルにoがない場合)
			if (!isGetO) {
				index++;
				modelData.resize(index + 1);
			}
			//各オブジェクト毎のマテリアル名を記憶させる
			modelData.at(index).materialName = materialName;
		}
	}
	//ファイルの読み直し
	std::ifstream file2(directoryPath + "/" + filename);
	assert(file.is_open());
	//mtlファイルを開いてマテリアル情報を得る(materialNameを基に)
	while (std::getline(file2, line2)) {
		std::string identifier;
		std::istringstream s(line2);
		s >> identifier;//←先頭の識別子を読む
		//identifierに応じた処理
		if (identifier == "mtllib") {
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			for (size_t index = 0; index < modelData.size(); index++) {
				//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を探す。
				modelData.at(index).material = LoadMaterialTemplateFile(directoryPath, materialFilename, modelData.at(index).materialName);
			}
			//読んだら抜ける
			break;
		}
	}
	//4.ModelDataを返す
	return modelData;
}

void RenderingPipeLine(
	TransformationMatrix* wvpData,
	const Transform transform,
	const Transform cameraTransform,
	const int32_t kClientWidth,
	const int32_t kClientHeight
) {
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	wvpData->WVP = worldViewProjectionMatrix;
	wvpData->World = worldMatrix;
}

void DrawModel(
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
	const ModelResource modelResource,
	const bool isDisplayModel
) {
	for (size_t index = 0; index < modelResource.modelData.size(); index++) {
		//モデルの描画
		commandList->IASetVertexBuffers(0, 1, &modelResource.vertexBufferView.at(index));
		//マテリアルCBufferの場所を設定
		commandList->SetGraphicsRootConstantBufferView(0, modelResource.materialResource.at(index)->GetGPUVirtualAddress());
		//モデルにテクスチャがない場合、スキップ
		if (modelResource.modelData.at(index).material.textureFilePath.size() != 0) {
			//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]でテクスチャの設定をしているため。
			commandList->SetGraphicsRootDescriptorTable(2, modelResource.textureSrvHandleGPU.at(index));
		}
		//wvp用のCBufferの場所を指定
		commandList->SetGraphicsRootConstantBufferView(1, modelResource.wvpResource->GetGPUVirtualAddress());
		//モデルの描画
		if (isDisplayModel) {
			commandList->DrawInstanced(UINT(modelResource.modelData.at(index).vertices.size()), 1, 0, 0);
		}
	}
}

void MakeModelResource(
	const char* resourceFileName,
	const char* objFileName,
	DirectXCommon* dxCommon,
	ModelResource& modelResource
) {
	modelResource.modelData = LoadObjFIle(resourceFileName, objFileName);
	const size_t kModelNum = modelResource.modelData.size();
	//std::vector型の要素数を確定
	modelResource.vertexResource.resize(kModelNum);
	modelResource.vertexBufferView.resize(kModelNum);
	modelResource.vertexData.resize(kModelNum);
	modelResource.materialResource.resize(kModelNum);
	modelResource.materialData.resize(kModelNum);
	modelResource.textureResorce.resize(kModelNum);
	modelResource.textureSrvHandleCPU.resize(kModelNum);
	modelResource.textureSrvHandleGPU.resize(kModelNum);
	modelResource.uvTransform.resize(kModelNum);
	for (size_t index = 0; index < kModelNum; index++) {
		//頂点用リソースを作る
		modelResource.vertexResource.at(index) = dxCommon->CreateBufferResource(sizeof(VertexData) * modelResource.modelData.at(index).vertices.size());
		//頂点バッファービューを作成
		modelResource.vertexBufferView.at(index).BufferLocation = modelResource.vertexResource.at(index)->GetGPUVirtualAddress();
		modelResource.vertexBufferView.at(index).SizeInBytes = UINT(sizeof(VertexData) * modelResource.modelData.at(index).vertices.size());
		modelResource.vertexBufferView.at(index).StrideInBytes = sizeof(VertexData);
		//頂点用リソースにデータを書き込む
		modelResource.vertexResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&modelResource.vertexData.at(index)));
		std::memcpy(modelResource.vertexData.at(index), modelResource.modelData.at(index).vertices.data(), sizeof(VertexData) * modelResource.modelData.at(index).vertices.size());
		//マテリアル用のリソースを作る。
		modelResource.materialResource.at(index) = dxCommon->CreateBufferResource(sizeof(Material));
		//書き込むためのアドレスを取得
		modelResource.materialResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&modelResource.materialData.at(index)));
		//白を書き込んでおく
		modelResource.materialData.at(index)->color = modelResource.modelData.at(index).material.colorData;
		//ライティング
		modelResource.materialData.at(index)->lightingKind = HalfLambert;
		//uvTransform
		modelResource.materialData.at(index)->uvTransform = MakeIdentity4x4();
		//テクスチャを持っているか
		bool isTexture = true;
		if (modelResource.modelData.at(index).material.textureFilePath.size() == 0) {
			//テクスチャファイルパスに書き込まれていない→テクスチャがない
			isTexture = false;
		}
		modelResource.materialData.at(index)->isTexture = isTexture;
		//UVトランスフォーム
		modelResource.uvTransform.at(index) = {
			{1.0f,1.0f,1.0f},
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f}
		};
	}
	//WVP用のリソースを作る。
	modelResource.wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	modelResource.wvpData = nullptr;
	//書き込むためのアドレスを取得
	modelResource.wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&modelResource.wvpData));
	//単位行列を書き込んでおく
	modelResource.wvpData->WVP = MakeIdentity4x4();
	modelResource.wvpData->World = MakeIdentity4x4();
	//トランスフォーム
	modelResource.transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};
}

//テクスチャ設定用関数
void SetTexture(
	ModelResource& modelResource,
	DirectXCommon* dxCommon,
	uint32_t& site
) {
	for (size_t i = 0; i < modelResource.modelData.size(); i++) {
		//もし、モデルのテクスチャがないなら、この処理はしない
		if (modelResource.modelData.at(i).material.textureFilePath.size() == 0) {
			break;
		}
		//モデル用のTextureを読んで転送する
		DirectX::ScratchImage mipImagesModel = DirectXCommon::LoadTexture(modelResource.modelData.at(i).material.textureFilePath);
		const DirectX::TexMetadata& metadataModel = mipImagesModel.GetMetadata();
		modelResource.textureResorce.at(i) = dxCommon->CreateTextureResource(metadataModel);
		dxCommon->UploadTextureData(modelResource.textureResorce.at(i), mipImagesModel);
		//metadataをもとにSRVの設定
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDescModel{};
		srvDescModel.Format = metadataModel.format;
		srvDescModel.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDescModel.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDescModel.Texture2D.MipLevels = UINT(metadataModel.mipLevels);
		//DescriptorHeapの場所移動
		site++;
		//SRVを作成するDescriptorHeapの場所を決める
		modelResource.textureSrvHandleCPU.at(i) = dxCommon->GetSRVCPUDescriptorHandle(site);
		modelResource.textureSrvHandleGPU.at(i) = dxCommon->GetSRVGPUDescriptorHandle(site);
		//SRVの作成
		dxCommon->GetDevice()->CreateShaderResourceView(modelResource.textureResorce.at(i).Get(), &srvDescModel, modelResource.textureSrvHandleCPU.at(i));
	}
}

//音声データの読み込み
SoundData SoundLoadWave(const char* filename)
{
	//1.ファイルオープン
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	assert(file.is_open());
	//2.「.wav」データ読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}
	FormatChunk format = {};
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	//3.ファイルクローズ
	file.close();
	//4.読み込んだ音声データをreturn
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

//音声データの解放
void SoundUnload(SoundData* soundData)
{
	//バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

//サウンドの再生
void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData)
{
	HRESULT result;
	//波形フォーマットからSourceVoiceを生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));
	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	//波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//解放処理確認用
	D3DResourceLeakChecker* leakChecker = nullptr;
	leakChecker = new D3DResourceLeakChecker();

	//ポインタ
	WinApp* winApp = nullptr;
	//WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	//ポインタ
	DirectXCommon* dxCommon = nullptr;
	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//ポインタ
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp);

	HRESULT hr;



	///////////////////////////////////////
	///////////////PSO START///////////////
	///////////////////////////////////////

	//RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//DescriptorRange作成
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	//マテリアルの設定
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0;//レジスタ番号0とバインド
	//オブジェクト関連の設定
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;//VertexShaderで使う
	rootParameters[1].Descriptor.ShaderRegister = 0;//レジスタ番号0とバインド
	//テクスチャの設定
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//Tableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	//平行光源用の設定
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;//CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;//レジスタ番号1とバインド

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
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatireBlob->GetBufferPointer(),
		signatireBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	//BlendSyayeの設定
	D3D12_BLEND_DESC blendDesc{};
	//全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//Shaderをコンパイルする

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxCommon->CompileShader(L"Resources/shaders/Object3D.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxCommon->CompileShader(L"Resources/shaders/Object3D.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	/////////////////////////////////////
	///////////////PSO END///////////////
	/////////////////////////////////////


	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature.Get();
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
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = dxCommon->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	//////////////////////////////////triangleのリソースを作る///////////////////////////////////////////////////////////////////
	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	//リソースの先頭アドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	///三角形1個目
	//左下
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	//上
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	//右下
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	///三角形2個目
	//左下2
	vertexData[3].position = { -0.5f,-0.5f,0.5f,1.0f };
	vertexData[3].texcoord = { 0.0f,1.0f };
	//上2
	vertexData[4].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData[4].texcoord = { 0.5f,0.0f };
	//右下2
	vertexData[5].position = { 0.5f,-0.5f,-0.5f,1.0f };
	vertexData[5].texcoord = { 1.0f,1.0f };

	for (UINT i = 0; i < 6; i++) {
		vertexData[i].normal.x = vertexData[i].position.x;
		vertexData[i].normal.y = vertexData[i].position.y;
		vertexData[i].normal.z = vertexData[i].position.z;
	}

	//マテリアル(色)用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxCommon->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は白を書き込んでみる
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//ライティングオフ
	materialData->lightingKind = NoneLighting;
	//uvTransformは単位行列を入れておく
	materialData->uvTransform = MakeIdentity4x4();
	//テクスチャを持っているか
	materialData->isTexture = true;

	//WVP用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpData = nullptr;
	//書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	//単位行列を書き込んでおく
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();
	///////////////////////////////////////////////////////////////////////////////////////////////





	//Transform変数を作る
	Transform transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};


	//カメラの位置、角度を作る
	Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-10.0f}
	};



	/////////////////////////Sphere用の頂点リソースを作る/////////////////////////////////////////////////////////////////////////////////
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere = dxCommon->CreateBufferResource(sizeof(VertexData) * (kSubdivision * kSubdivision * 6));
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	//リソースの先端のアドレスから使う
	vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * (kSubdivision * kSubdivision * 6);
	//1頂点あたりのサイズ
	vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexDataSphere = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));
	///頂点の座標を決めていく
	//経度分割1つ分の角度
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	//緯度分割1つ分の角度
	const float kLatEvery = pi / float(kSubdivision);
	//球の情報
	Sphere sphere;
	sphere.center = { 0.0f,0.0f,0.0f };
	sphere.radius = 1.0f;
	//緯度の方向に分割
	for (UINT latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;
		//緯度の方向に分割
		for (UINT lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;
			///三角形一枚目
			//頂点にデータを入れる
			vertexDataSphere[start].position.x = sphere.center.x + sphere.radius * cos(lat) * cos(lon);
			vertexDataSphere[start].position.y = sphere.center.y + sphere.radius * sin(lat);
			vertexDataSphere[start].position.z = sphere.center.z + sphere.radius * cos(lat) * sin(lon);
			vertexDataSphere[start].position.w = 1.0f;
			vertexDataSphere[start].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexDataSphere[start].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			//頂点にデータを入れる
			vertexDataSphere[start + 1].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon);
			vertexDataSphere[start + 1].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
			vertexDataSphere[start + 1].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon);
			vertexDataSphere[start + 1].position.w = 1.0f;
			vertexDataSphere[start + 1].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexDataSphere[start + 1].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			//頂点にデータを入れる
			vertexDataSphere[start + 2].position.x = sphere.center.x + sphere.radius * cos(lat) * cos(lon + kLonEvery);
			vertexDataSphere[start + 2].position.y = sphere.center.y + sphere.radius * sin(lat);
			vertexDataSphere[start + 2].position.z = sphere.center.z + sphere.radius * cos(lat) * sin(lon + kLonEvery);
			vertexDataSphere[start + 2].position.w = 1.0f;
			vertexDataSphere[start + 2].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexDataSphere[start + 2].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			///三角形二枚目
			//頂点にデータを入れる
			vertexDataSphere[start + 3].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon);
			vertexDataSphere[start + 3].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
			vertexDataSphere[start + 3].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon);
			vertexDataSphere[start + 3].position.w = 1.0f;
			vertexDataSphere[start + 3].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexDataSphere[start + 3].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			//頂点にデータを入れる
			vertexDataSphere[start + 4].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon + kLonEvery);
			vertexDataSphere[start + 4].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
			vertexDataSphere[start + 4].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon + kLonEvery);
			vertexDataSphere[start + 4].position.w = 1.0f;
			vertexDataSphere[start + 4].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexDataSphere[start + 4].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			//頂点にデータを入れる
			vertexDataSphere[start + 5].position.x = sphere.center.x + sphere.radius * cos(lat) * cos(lon + kLonEvery);
			vertexDataSphere[start + 5].position.y = sphere.center.y + sphere.radius * sin(lat);
			vertexDataSphere[start + 5].position.z = sphere.center.z + sphere.radius * cos(lat) * sin(lon + kLonEvery);
			vertexDataSphere[start + 5].position.w = 1.0f;
			vertexDataSphere[start + 5].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexDataSphere[start + 5].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);

			//法線情報の入力
			for (uint32_t i = 0; i < 6; i++) {
				vertexDataSphere[start + i].normal.x = vertexDataSphere[start + i].position.x;
				vertexDataSphere[start + i].normal.y = vertexDataSphere[start + i].position.y;
				vertexDataSphere[start + i].normal.z = vertexDataSphere[start + i].position.z;
			}

			//メモ//
			//三角形描画時には頂点を半時計周りの順番で設定する。
			//時計回りにすると表裏が逆になってしまう。
		}
	}
	//マテリアル用のリソースを作る。
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSphere = dxCommon->CreateBufferResource(sizeof(Material));
	//データを書き込む
	Material* materialDataSphere = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	//白を書き込んでおく
	materialDataSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//ライティングオン
	materialDataSphere->lightingKind = HalfLambert;
	//uvTransform
	materialDataSphere->uvTransform = MakeIdentity4x4();
	//テクスチャを持っているか
	materialDataSphere->isTexture = true;
	//WVP用のリソースを作る。
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResourceSphere = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpDataSphere = nullptr;
	//書き込むためのアドレスを取得
	wvpResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSphere));
	//単位行列を書き込んでおく
	wvpDataSphere->WVP = MakeIdentity4x4();
	wvpDataSphere->World = MakeIdentity4x4();
	//トランスフォーム
	Transform transformSphere = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////Sprite用の頂点リソースを作る////////////////////////////////////////////
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//リソースの先端のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
	//頂点リソースにデータを書き込む
	VertexData* vertexDataSprite = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	///三角形1個目
	vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[2].texcoord = { 1.0f,1.0f };
	///三角形2個目
	vertexDataSprite[3].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[3].texcoord = { 0.0f,0.0f };
	vertexDataSprite[4].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexDataSprite[4].texcoord = { 1.0f,0.0f };
	vertexDataSprite[5].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDataSprite[5].texcoord = { 1.0f,1.0f };
	//法線情報
	for (UINT i = 0; i < 6; i++) {
		vertexDataSprite[i].normal = { 0.0f,0.0f,-1.0f };
	}
	//sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxCommon->CreateBufferResource(sizeof(Material));
	//データを書き込む
	Material* materialDataSprite = nullptr;
	//書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//白を書き込んでおく
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//ライティングオフ
	materialDataSprite->lightingKind = NoneLighting;
	//uvTransformは単位行列を入れておく
	materialDataSprite->uvTransform = MakeIdentity4x4();
	//テクスチャを持っているか
	materialDataSprite->isTexture = true;
	//Sprite用のTransformationMatrix用のリソースを作る。
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->World = MakeIdentity4x4();
	//CPU(ImGui)で動かす用のTransform
	Transform transformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};
	//uvTransform用の変数
	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	////////////////////////////////////////////////////////////////////////////////////////

	////////////////////平行光源のリソースを作る////////////////////////////////////
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	//データを作る
	DirectionalLight* directionalLightData = nullptr;
	//書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//データに書き込む
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;
	////////////////////////////////////////////////////////////////////////////////////////

	///////////////////インデックス用リソースを作る////////////////////////////////////
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);
	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//リソースの先端のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;
	//インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;


	/////////////////////////モデル用のリソースを作る/////////////////////////////////////////////////////////////////////////////////
	ModelResource modelResource;
	MakeModelResource("Resources", "axis.obj", dxCommon, modelResource);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////モデル2のリソース////////////////////////////////////////////////////
	ModelResource model2Resource;
	MakeModelResource("Resources", "plane.obj", dxCommon, model2Resource);
	///////////////////////////////////////////////////////////////////////////////

	/////////////////////////Utah TeaPotのリソースを作る/////////////////////////////////////////////////////////////////////////////////
	ModelResource model3Resource;
	MakeModelResource("Resources", "teapot.obj", dxCommon, model3Resource);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////Stanford Bunnyのリソースを作る/////////////////////////////////
	ModelResource model4Resource;
	MakeModelResource("Resources", "bunny.obj", dxCommon, model4Resource);
	/////////////////////////////////////////////////////////////////////////

	///////////////////MultiMeshのリソースを作る/////////////////////////////////
	ModelResource model5Resource;
	MakeModelResource("Resources", "multiMesh.obj", dxCommon, model5Resource);
	/////////////////////////////////////////////////////////////////////////

	///////////////////MultiMaterialのリソースを作る/////////////////////////////////
	ModelResource model6Resource;
	MakeModelResource("Resources", "multiMaterial.obj", dxCommon, model6Resource);
	/////////////////////////////////////////////////////////////////////////

	///////////////////Suzanneのリソースを作る/////////////////////////////////
	ModelResource model7Resource;
	MakeModelResource("Resources", "suzanne.obj", dxCommon, model7Resource);
	/////////////////////////////////////////////////////////////////////////

	///////////////////Suzanne2のリソースを作る/////////////////////////////////
	ModelResource model8Resource;
	MakeModelResource("Resources", "suzanne.obj", dxCommon, model8Resource);
	/////////////////////////////////////////////////////////////////////////


	////////////////////////Textureの設定///////////////////////////////////
	//DescriptorHeap配置場所
	uint32_t site = 0;
	//Textureを読んで転送する
	DirectX::ScratchImage mipImages = DirectXCommon::LoadTexture("Resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResorce = dxCommon->CreateTextureResource(metadata);
	dxCommon->UploadTextureData(textureResorce.Get(), mipImages);
	//metadataをもとにSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//DescriptorHeapの場所移動
	site++;
	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(site);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(site);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResorce.Get(), &srvDesc, textureSrvHandleCPU);

	//2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = DirectXCommon::LoadTexture("Resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResorce2 = dxCommon->CreateTextureResource(metadata2);
	dxCommon->UploadTextureData(textureResorce2.Get(), mipImages2);
	//metadataをもとにSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);
	//DescriptorHeapの場所移動
	site++;
	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetSRVCPUDescriptorHandle(site);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetSRVGPUDescriptorHandle(site);
	//SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(textureResorce2.Get(), &srvDesc2, textureSrvHandleCPU2);

	//モデル用のTextureを読んで転送する
	SetTexture(modelResource, dxCommon, site);

	//モデル2用のTextureを読んで転送する
	SetTexture(model2Resource, dxCommon, site);

	//モデル3用のTextureを読んで転送する
	SetTexture(model3Resource, dxCommon, site);

	//モデル4用のTextureを読んで転送する
	SetTexture(model4Resource, dxCommon, site);

	//モデル5用のTextureを読んで転送する
	SetTexture(model5Resource, dxCommon, site);

	//モデル6用のTextureを読んで転送する
	SetTexture(model6Resource, dxCommon, site);

	//モデル7用のTextureを読んで転送する
	SetTexture(model7Resource, dxCommon, site);

	//モデル8用のTextureを読んで転送する
	SetTexture(model8Resource, dxCommon, site);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////Xaudio2の設定/////////////////////////////////////
	//必要な変数の宣言
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;
	///////////////////////////////////////////////////////////////////


	//初期化
	//表示
	bool isDisplayTriangle = false;
	bool isDisplaySprite = false;
	bool isDisplaySphere = false;
	bool isDisplayIndex = false;
	bool isDisplayModel = false;
	bool isDisplayModel2 = false;
	bool isDisplayModel3 = false;
	bool isDisplayModel4 = false;
	bool isDisplayModel5 = false;
	bool isDisplayModel6 = false;
	bool isDisplayModel7 = false;
	bool isDisplayModel8 = false;
	bool useMonsterBall = false;
	//ライト
	bool isLightingSphere = true;
	bool isHalfLambert = true;
	//XAudio2エンジンのインスタンス作成
	hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	//マスターボイスを作成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));
	//音声読み込み
	SoundData soundData1 = SoundLoadWave("Resources/Alarm01.wav");
	//再生フラグ
	bool isPlayAudio = false;


	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		//メッセージ処理
		if (winApp->ProcessMessage()) {
			break;
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		input->Update();


		//ゲームの処理

		//////////////////////
		///更新処理
		//////////////////////




		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える

		ImGui::Begin("Settings");
		//テクスチャ
		if (ImGui::TreeNode("texture")) {
			ImGui::Checkbox("useMonsterBall", &useMonsterBall);

			ImGui::TreePop();
		}
		//スプライト
		if (ImGui::TreeNode("Sprite")) {
			//スプライトの平行移動
			ImGui::DragFloat3("Transform", &transformSprite.translate.x, 1.0f);
			//uvTransform
			ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

			//パラメーターの更新
			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDataSprite->uvTransform = uvTransformMatrix;

			//リセット
			if (ImGui::Button("reset")) {
				transformSprite.translate = { 0.0f,0.0f,0.0f };
				uvTransformSprite.translate = { 0.0f,0.0f,0.0f };
				uvTransformSprite.scale = { 1.0f,1.0f,1.0f };
				uvTransformSprite.rotate = { 0.0f,0.0f,0.0f };
			}
			//スプライトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplaySprite = !isDisplaySprite;
			}

			ImGui::TreePop();
		}
		//三角形
		if (ImGui::TreeNode("Triangle")) {
			//オブジェクトの平行移動
			ImGui::DragFloat3("translate", &transform.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &transform.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &transform.scale.x, 0.01f);
			//リセット
			if (ImGui::Button("reset")) {
				transform.translate = { 0.0f,0.0f,0.0f };
				transform.rotate = { 0.0f,0.0f,0.0f };
				transform.scale = { 1.0f,1.0f,1.0f };
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayTriangle = !isDisplayTriangle;
			}

			ImGui::TreePop();
		}
		//球
		if (ImGui::TreeNode("Sphere")) {
			//オブジェクトの平行移動
			ImGui::DragFloat3("translate", &transformSphere.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &transformSphere.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &transformSphere.scale.x, 0.01f);
			//ライティング

			//リセット
			if (ImGui::Button("reset")) {
				transformSphere.translate = { 0.0f,0.0f,0.0f };
				transformSphere.rotate = { 0.0f,0.0f,0.0f };
				transformSphere.scale = { 1.0f,1.0f,1.0f };
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplaySphere = !isDisplaySphere;
			}

			ImGui::TreePop();
		}
		//平行光源
		if (ImGui::TreeNode("Light")) {
			//色
			ImGui::DragFloat4("color", &directionalLightData->color.x, 0.01f);
			//方向
			ImGui::DragFloat3("direction", &directionalLightData->direction.x, 0.01f);
			Normalize(directionalLightData->direction);
			//輝度
			ImGui::DragFloat("intensity", &directionalLightData->intensity, 0.01f);

			//リセット
			if (ImGui::Button("reset")) {
				directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
				directionalLightData->direction = { 0.0f,-1.0f,0.0f };
				directionalLightData->intensity = 1.0f;
			}

			ImGui::TreePop();
		}
		//インデックス三角形
		if (ImGui::TreeNode("index")) {
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayIndex = !isDisplayIndex;
			}

			ImGui::TreePop();
		}
		//モデル
		if (ImGui::TreeNode("Axis")) {
			//オブジェクトの平行移動
			ImGui::DragFloat3("translate", &modelResource.transform.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &modelResource.transform.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &modelResource.transform.scale.x, 0.01f);
			//マテリアル設定
			for (size_t index = 0; index < modelResource.modelData.size(); index++) {
				std::string material = "Material";
				std::string strIndex = material + std::to_string(index + 1);
				if (ImGui::TreeNode(strIndex.c_str())) {
					//UVトランスフォーム
					ImGui::DragFloat2("UVTranslate", &modelResource.uvTransform.at(index).translate.x, 0.01f, -10.0f, 10.0f);
					ImGui::DragFloat2("UVScale", &modelResource.uvTransform.at(index).scale.x, 0.01f, -10.0f, 10.0f);
					ImGui::SliderAngle("UVRotate", &modelResource.uvTransform.at(index).rotate.z);
					//パラメーターの更新
					Matrix4x4 uvTransformMatrix = MakeScaleMatrix(modelResource.uvTransform.at(index).scale);
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(modelResource.uvTransform.at(index).rotate.z));
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(modelResource.uvTransform.at(index).translate));
					modelResource.materialData.at(index)->uvTransform = uvTransformMatrix;
					//カラー変更
					ImGui::ColorEdit4("color", &modelResource.materialData.at(index)->color.x, 0.01f);
					//ライティング変更
					const char* allLightKind[] = { "HalfLambert","Lambert","NoneLighting" };
					ImGui::Combo("lighting", &modelResource.materialData.at(index)->lightingKind, allLightKind, IM_ARRAYSIZE(allLightKind));

					ImGui::TreePop();
				}
			}
			//リセット
			if (ImGui::Button("reset")) {
				modelResource.transform.translate = { 0.0f,0.0f,0.0f };
				modelResource.transform.rotate = { 0.0f,0.0f,0.0f };
				modelResource.transform.scale = { 1.0f,1.0f,1.0f };
				//マテリアル
				for (size_t index = 0; index < modelResource.modelData.size(); index++) {
					modelResource.uvTransform.at(index).translate = { 0.0f,0.0f,0.0f };
					modelResource.uvTransform.at(index).rotate = { 0.0f,0.0f,0.0f };
					modelResource.uvTransform.at(index).scale = { 1.0f,1.0f,1.0f };
					modelResource.materialData.at(index)->color = modelResource.modelData.at(index).material.colorData;
					modelResource.materialData.at(index)->lightingKind = HalfLambert;
				}
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayModel = !isDisplayModel;
			}

			ImGui::TreePop();
		}
		//モデル2
		if (ImGui::TreeNode("Plane")) {
			//オブジェクトの平行移動
			ImGui::DragFloat3("translate", &model2Resource.transform.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &model2Resource.transform.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &model2Resource.transform.scale.x, 0.01f);
			//マテリアル設定
			for (size_t index = 0; index < model2Resource.modelData.size(); index++) {
				std::string material = "Material";
				std::string strIndex = material + std::to_string(index + 1);
				if (ImGui::TreeNode(strIndex.c_str())) {
					//UVトランスフォーム
					ImGui::DragFloat2("UVTranslate", &model2Resource.uvTransform.at(index).translate.x, 0.01f, -10.0f, 10.0f);
					ImGui::DragFloat2("UVScale", &model2Resource.uvTransform.at(index).scale.x, 0.01f, -10.0f, 10.0f);
					ImGui::SliderAngle("UVRotate", &model2Resource.uvTransform.at(index).rotate.z);
					//パラメーターの更新
					Matrix4x4 uvTransformMatrix = MakeScaleMatrix(model2Resource.uvTransform.at(index).scale);
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(model2Resource.uvTransform.at(index).rotate.z));
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(model2Resource.uvTransform.at(index).translate));
					model2Resource.materialData.at(index)->uvTransform = uvTransformMatrix;
					//カラー変更
					ImGui::ColorEdit4("color", &model2Resource.materialData.at(index)->color.x, 0.01f);
					//ライティング変更
					const char* allLightKind[] = { "HalfLambert","Lambert","NoneLighting" };
					ImGui::Combo("lighting", &model2Resource.materialData.at(index)->lightingKind, allLightKind, IM_ARRAYSIZE(allLightKind));

					ImGui::TreePop();
				}
			}
			//リセット
			if (ImGui::Button("reset")) {
				model2Resource.transform.translate = { 0.0f,0.0f,0.0f };
				model2Resource.transform.rotate = { 0.0f,0.0f,0.0f };
				model2Resource.transform.scale = { 1.0f,1.0f,1.0f };
				//マテリアル
				for (size_t index = 0; index < model2Resource.modelData.size(); index++) {
					model2Resource.uvTransform.at(index).translate = { 0.0f,0.0f,0.0f };
					model2Resource.uvTransform.at(index).rotate = { 0.0f,0.0f,0.0f };
					model2Resource.uvTransform.at(index).scale = { 1.0f,1.0f,1.0f };
					model2Resource.materialData.at(index)->color = model2Resource.modelData.at(index).material.colorData;
					model2Resource.materialData.at(index)->lightingKind = HalfLambert;
				}
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayModel2 = !isDisplayModel2;
			}

			ImGui::TreePop();
		}
		//モデル3
		if (ImGui::TreeNode("UtahTeapot")) {
			//オブジェクトの平行移動
			ImGui::DragFloat3("translate", &model3Resource.transform.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &model3Resource.transform.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &model3Resource.transform.scale.x, 0.01f);
			//マテリアル設定
			for (size_t index = 0; index < model3Resource.modelData.size(); index++) {
				std::string material = "Material";
				std::string strIndex = material + std::to_string(index + 1);
				if (ImGui::TreeNode(strIndex.c_str())) {
					//UVトランスフォーム
					ImGui::DragFloat2("UVTranslate", &model3Resource.uvTransform.at(index).translate.x, 0.01f, -10.0f, 10.0f);
					ImGui::DragFloat2("UVScale", &model3Resource.uvTransform.at(index).scale.x, 0.01f, -10.0f, 10.0f);
					ImGui::SliderAngle("UVRotate", &model3Resource.uvTransform.at(index).rotate.z);
					//パラメーターの更新
					Matrix4x4 uvTransformMatrix = MakeScaleMatrix(model3Resource.uvTransform.at(index).scale);
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(model3Resource.uvTransform.at(index).rotate.z));
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(model3Resource.uvTransform.at(index).translate));
					model3Resource.materialData.at(index)->uvTransform = uvTransformMatrix;
					//カラー変更
					ImGui::ColorEdit4("color", &model3Resource.materialData.at(index)->color.x, 0.01f);
					//ライティング変更
					const char* allLightKind[] = { "HalfLambert","Lambert","NoneLighting" };
					ImGui::Combo("lighting", &model3Resource.materialData.at(index)->lightingKind, allLightKind, IM_ARRAYSIZE(allLightKind));

					ImGui::TreePop();
				}
			}
			//リセット
			if (ImGui::Button("reset")) {
				model3Resource.transform.translate = { 0.0f,0.0f,0.0f };
				model3Resource.transform.rotate = { 0.0f,0.0f,0.0f };
				model3Resource.transform.scale = { 1.0f,1.0f,1.0f };
				//マテリアル
				for (size_t index = 0; index < model3Resource.modelData.size(); index++) {
					model3Resource.uvTransform.at(index).translate = { 0.0f,0.0f,0.0f };
					model3Resource.uvTransform.at(index).rotate = { 0.0f,0.0f,0.0f };
					model3Resource.uvTransform.at(index).scale = { 1.0f,1.0f,1.0f };
					model3Resource.materialData.at(index)->color = model3Resource.modelData.at(index).material.colorData;
					model3Resource.materialData.at(index)->lightingKind = HalfLambert;
				}
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayModel3 = !isDisplayModel3;
			}

			ImGui::TreePop();
		}
		//モデル4
		if (ImGui::TreeNode("StanfordBunny")) {
			//オブジェクトの平行移動
			ImGui::DragFloat3("translate", &model4Resource.transform.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &model4Resource.transform.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &model4Resource.transform.scale.x, 0.01f);
			//マテリアル設定
			for (size_t index = 0; index < model4Resource.modelData.size(); index++) {
				std::string material = "Material";
				std::string strIndex = material + std::to_string(index + 1);
				if (ImGui::TreeNode(strIndex.c_str())) {
					//UVトランスフォーム
					ImGui::DragFloat2("UVTranslate", &model4Resource.uvTransform.at(index).translate.x, 0.01f, -10.0f, 10.0f);
					ImGui::DragFloat2("UVScale", &model4Resource.uvTransform.at(index).scale.x, 0.01f, -10.0f, 10.0f);
					ImGui::SliderAngle("UVRotate", &model4Resource.uvTransform.at(index).rotate.z);
					//パラメーターの更新
					Matrix4x4 uvTransformMatrix = MakeScaleMatrix(model4Resource.uvTransform.at(index).scale);
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(model4Resource.uvTransform.at(index).rotate.z));
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(model4Resource.uvTransform.at(index).translate));
					model4Resource.materialData.at(index)->uvTransform = uvTransformMatrix;
					//カラー変更
					ImGui::ColorEdit4("color", &model4Resource.materialData.at(index)->color.x, 0.01f);
					//ライティング変更
					const char* allLightKind[] = { "HalfLambert","Lambert","NoneLighting" };
					ImGui::Combo("lighting", &model4Resource.materialData.at(index)->lightingKind, allLightKind, IM_ARRAYSIZE(allLightKind));

					ImGui::TreePop();
				}
			}
			//リセット
			if (ImGui::Button("reset")) {
				model4Resource.transform.translate = { 0.0f,0.0f,0.0f };
				model4Resource.transform.rotate = { 0.0f,0.0f,0.0f };
				model4Resource.transform.scale = { 1.0f,1.0f,1.0f };
				//マテリアル
				for (size_t index = 0; index < model4Resource.modelData.size(); index++) {
					model4Resource.uvTransform.at(index).translate = { 0.0f,0.0f,0.0f };
					model4Resource.uvTransform.at(index).rotate = { 0.0f,0.0f,0.0f };
					model4Resource.uvTransform.at(index).scale = { 1.0f,1.0f,1.0f };
					model4Resource.materialData.at(index)->color = model4Resource.modelData.at(index).material.colorData;
					model4Resource.materialData.at(index)->lightingKind = HalfLambert;
				}
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayModel4 = !isDisplayModel4;
			}

			ImGui::TreePop();
		}
		//モデル5
		if (ImGui::TreeNode("MultiMesh")) {
			//オブジェクトの平行移動
			ImGui::DragFloat3("translate", &model5Resource.transform.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &model5Resource.transform.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &model5Resource.transform.scale.x, 0.01f);
			//マテリアル設定
			for (size_t index = 0; index < model5Resource.modelData.size(); index++) {
				std::string material = "Material";
				std::string strIndex = material + std::to_string(index + 1);
				if (ImGui::TreeNode(strIndex.c_str())) {
					//UVトランスフォーム
					ImGui::DragFloat2("UVTranslate", &model5Resource.uvTransform.at(index).translate.x, 0.01f, -10.0f, 10.0f);
					ImGui::DragFloat2("UVScale", &model5Resource.uvTransform.at(index).scale.x, 0.01f, -10.0f, 10.0f);
					ImGui::SliderAngle("UVRotate", &model5Resource.uvTransform.at(index).rotate.z);
					//パラメーターの更新
					Matrix4x4 uvTransformMatrix = MakeScaleMatrix(model5Resource.uvTransform.at(index).scale);
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(model5Resource.uvTransform.at(index).rotate.z));
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(model5Resource.uvTransform.at(index).translate));
					model5Resource.materialData.at(index)->uvTransform = uvTransformMatrix;
					//カラー変更
					ImGui::ColorEdit4("color", &model5Resource.materialData.at(index)->color.x, 0.01f);
					//ライティング変更
					const char* allLightKind[] = { "HalfLambert","Lambert","NoneLighting" };
					ImGui::Combo("lighting", &model5Resource.materialData.at(index)->lightingKind, allLightKind, IM_ARRAYSIZE(allLightKind));

					ImGui::TreePop();
				}
			}
			//リセット
			if (ImGui::Button("reset")) {
				model5Resource.transform.translate = { 0.0f,0.0f,0.0f };
				model5Resource.transform.rotate = { 0.0f,0.0f,0.0f };
				model5Resource.transform.scale = { 1.0f,1.0f,1.0f };
				//マテリアル
				for (size_t index = 0; index < model5Resource.modelData.size(); index++) {
					model5Resource.uvTransform.at(index).translate = { 0.0f,0.0f,0.0f };
					model5Resource.uvTransform.at(index).rotate = { 0.0f,0.0f,0.0f };
					model5Resource.uvTransform.at(index).scale = { 1.0f,1.0f,1.0f };
					model5Resource.materialData.at(index)->color = model5Resource.modelData.at(index).material.colorData;
					model5Resource.materialData.at(index)->lightingKind = HalfLambert;
				}
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayModel5 = !isDisplayModel5;
			}

			ImGui::TreePop();
		}
		//モデル6
		if (ImGui::TreeNode("MultiMaterial")) {
			//オブジェクトの平行移動
			ImGui::DragFloat3("translate", &model6Resource.transform.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &model6Resource.transform.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &model6Resource.transform.scale.x, 0.01f);
			//マテリアル設定
			for (size_t index = 0; index < model6Resource.modelData.size(); index++) {
				std::string material = "Material";
				std::string strIndex = material + std::to_string(index + 1);
				if (ImGui::TreeNode(strIndex.c_str())) {
					//UVトランスフォーム
					ImGui::DragFloat2("UVTranslate", &model6Resource.uvTransform.at(index).translate.x, 0.01f, -10.0f, 10.0f);
					ImGui::DragFloat2("UVScale", &model6Resource.uvTransform.at(index).scale.x, 0.01f, -10.0f, 10.0f);
					ImGui::SliderAngle("UVRotate", &model6Resource.uvTransform.at(index).rotate.z);
					//パラメーターの更新
					Matrix4x4 uvTransformMatrix = MakeScaleMatrix(model6Resource.uvTransform.at(index).scale);
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(model6Resource.uvTransform.at(index).rotate.z));
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(model6Resource.uvTransform.at(index).translate));
					model6Resource.materialData.at(index)->uvTransform = uvTransformMatrix;
					//カラー変更
					ImGui::ColorEdit4("color", &model6Resource.materialData.at(index)->color.x, 0.01f);
					//ライティング変更
					const char* allLightKind[] = { "HalfLambert","Lambert","NoneLighting" };
					ImGui::Combo("lighting", &model6Resource.materialData.at(index)->lightingKind, allLightKind, IM_ARRAYSIZE(allLightKind));

					ImGui::TreePop();
				}
			}
			//リセット
			if (ImGui::Button("reset")) {
				model6Resource.transform.translate = { 0.0f,0.0f,0.0f };
				model6Resource.transform.rotate = { 0.0f,0.0f,0.0f };
				model6Resource.transform.scale = { 1.0f,1.0f,1.0f };
				//マテリアル
				for (size_t index = 0; index < model6Resource.modelData.size(); index++) {
					model6Resource.uvTransform.at(index).translate = { 0.0f,0.0f,0.0f };
					model6Resource.uvTransform.at(index).rotate = { 0.0f,0.0f,0.0f };
					model6Resource.uvTransform.at(index).scale = { 1.0f,1.0f,1.0f };
					model6Resource.materialData.at(index)->color = model6Resource.modelData.at(index).material.colorData;
					model6Resource.materialData.at(index)->lightingKind = HalfLambert;
				}
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayModel6 = !isDisplayModel6;
			}

			ImGui::TreePop();
		}
		//モデル7
		if (ImGui::TreeNode("Suzanne")) {
			//オブジェクトの平行移動
			ImGui::DragFloat3("translate", &model7Resource.transform.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &model7Resource.transform.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &model7Resource.transform.scale.x, 0.01f);
			//マテリアル設定
			for (size_t index = 0; index < model7Resource.modelData.size(); index++) {
				std::string material = "Material";
				std::string strIndex = material + std::to_string(index + 1);
				if (ImGui::TreeNode(strIndex.c_str())) {
					//UVトランスフォーム
					ImGui::DragFloat2("UVTranslate", &model7Resource.uvTransform.at(index).translate.x, 0.01f, -10.0f, 10.0f);
					ImGui::DragFloat2("UVScale", &model7Resource.uvTransform.at(index).scale.x, 0.01f, -10.0f, 10.0f);
					ImGui::SliderAngle("UVRotate", &model7Resource.uvTransform.at(index).rotate.z);
					//パラメーターの更新
					Matrix4x4 uvTransformMatrix = MakeScaleMatrix(model7Resource.uvTransform.at(index).scale);
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(model7Resource.uvTransform.at(index).rotate.z));
					uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(model7Resource.uvTransform.at(index).translate));
					model7Resource.materialData.at(index)->uvTransform = uvTransformMatrix;
					//カラー変更
					ImGui::ColorEdit4("color", &model7Resource.materialData.at(index)->color.x, 0.01f);
					//ライティング変更
					const char* allLightKind[] = { "HalfLambert","Lambert","NoneLighting" };
					ImGui::Combo("lighting", &model7Resource.materialData.at(index)->lightingKind, allLightKind, IM_ARRAYSIZE(allLightKind));

					ImGui::TreePop();
				}
			}
			//リセット
			if (ImGui::Button("reset")) {
				model7Resource.transform.translate = { 0.0f,0.0f,0.0f };
				model7Resource.transform.rotate = { 0.0f,0.0f,0.0f };
				model7Resource.transform.scale = { 1.0f,1.0f,1.0f };
				//マテリアル
				for (size_t index = 0; index < model7Resource.modelData.size(); index++) {
					model7Resource.uvTransform.at(index).translate = { 0.0f,0.0f,0.0f };
					model7Resource.uvTransform.at(index).rotate = { 0.0f,0.0f,0.0f };
					model7Resource.uvTransform.at(index).scale = { 1.0f,1.0f,1.0f };
					model7Resource.materialData.at(index)->color = model7Resource.modelData.at(index).material.colorData;
					model7Resource.materialData.at(index)->lightingKind = HalfLambert;
				}
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayModel7 = !isDisplayModel7;
			}


			ImGui::TreePop();
		}
		//モデル8
		if (ImGui::TreeNode("Input")) {
			//オブジェクトの平行移動
			if (input->PushKey(DIK_UP)) {
				model8Resource.transform.translate.y += 0.05f;
			}
			if (input->PushKey(DIK_DOWN)) {
				model8Resource.transform.translate.y -= 0.05f;
			}
			if (input->PushKey(DIK_RIGHT)) {
				model8Resource.transform.translate.x += 0.05f;
			}
			if (input->PushKey(DIK_LEFT)) {
				model8Resource.transform.translate.x -= 0.05f;
			}
			//リセット
			if (ImGui::Button("reset")) {
				model8Resource.transform.translate = { 0.0f,0.0f,0.0f };
				model8Resource.transform.rotate = { 0.0f,0.0f,0.0f };
				model8Resource.transform.scale = { 1.0f,1.0f,1.0f };
			}
			//オブジェクトの表示切り替え
			if (ImGui::Button("DisplayChange")) {
				isDisplayModel8 = !isDisplayModel8;
			}
			ImGui::TreePop();
		}
		//サウンド
		if (ImGui::TreeNode("Sound")) {
			if (ImGui::Button("PlayButton")) {
				isPlayAudio = !isPlayAudio;
			}

			if (isPlayAudio) {
				//音声再生
				SoundPlayWave(xAudio2.Get(), soundData1);
				isPlayAudio = false;
			}


			ImGui::TreePop();
		}


		ImGui::End();

		transform.rotate.y += 0.02f;
		transformSphere.rotate.y += 0.02f;
		modelResource.transform.rotate.y += 0.02f;
		model2Resource.transform.rotate.y += 0.02f;
		model3Resource.transform.rotate.y += 0.02f;
		model4Resource.transform.rotate.y += 0.02f;
		model5Resource.transform.rotate.y += 0.02f;
		model6Resource.transform.rotate.y += 0.02f;
		model7Resource.transform.rotate.y += 0.02f;
		model8Resource.transform.rotate.y += 0.02f;



		/////レンダリングパイプライン/////
		//各種行列の計算
		RenderingPipeLine(wvpData, transform, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);
		//Sprite用のWorldViewProjectionMatrixを作る
		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, (float)WinApp::kClientWidth, (float)WinApp::kClientHeight, 0.0f, 100.0f);
		Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
		transformationMatrixDataSprite->World = worldMatrixSprite;
		//Sphere用のWorldViewProjectionMatrixを作る
		RenderingPipeLine(wvpDataSphere, transformSphere, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);
		//モデル用のWorldViewProjectionMatrixを作る
		RenderingPipeLine(modelResource.wvpData, modelResource.transform, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);
		//モデル2用のWorldViewProjectionMatrixを作る
		RenderingPipeLine(model2Resource.wvpData, model2Resource.transform, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);
		//モデル3用のWorldViewProjectionMatrixを作る
		RenderingPipeLine(model3Resource.wvpData, model3Resource.transform, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);
		//モデル4用のWorldViewProjectionMatrixを作る
		RenderingPipeLine(model4Resource.wvpData, model4Resource.transform, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);
		//モデル5用のWorldViewProjectionMatrixを作る
		RenderingPipeLine(model5Resource.wvpData, model5Resource.transform, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);
		//モデル6用のWorldViewProjectionMatrixを作る
		RenderingPipeLine(model6Resource.wvpData, model6Resource.transform, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);
		//モデル7用のWorldViewProjectionMatrixを作る
		RenderingPipeLine(model7Resource.wvpData, model7Resource.transform, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);
		//モデル8用のWorldViewProjectionMatrixを作る
		RenderingPipeLine(model8Resource.wvpData, model8Resource.transform, cameraTransform, WinApp::kClientWidth, WinApp::kClientHeight);

		//ImGuiの内部コマンドを生成する
		ImGui::Render();

		//////////////////////
		///描画処理
		//////////////////////

		//描画前処理
		dxCommon->PreDraw();

		//RootSignatureを設定。PSOに設定しているけど別途設定が必要
		dxCommon->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
		dxCommon->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());

		//平行光源の設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		//triangleの描画
		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
		//形状を設定
		dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//マテリアルCBufferの場所を設定(0はrootparameterの0番目でマテリアルの設定してるため)
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]でテクスチャの設定をしているため。
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//wvp用のCBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
		//三角形の描画
		if (isDisplayTriangle) {
			dxCommon->GetCommandList()->DrawInstanced(6, 1, 0, 0);
		}

		//Sphereの描画
		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);
		//マテリアルCBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress());
		//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]でテクスチャの設定をしているため。
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
		//wvp用のCBufferの場所を指定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResourceSphere->GetGPUVirtualAddress());
		//球の描画
		if (isDisplaySphere) {
			dxCommon->GetCommandList()->DrawInstanced((kSubdivision * kSubdivision * 6), 1, 0, 0);
		}

		//Spriteの描画。変更が必要な物だけ変更する
		dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		//マテリアルCBufferの場所を設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		//テクスチャ設定
		dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		//TransformationMatrixCBufferの場所を指定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
		//描画！
		if (isDisplaySprite) {
			dxCommon->GetCommandList()->DrawInstanced(6, 1, 0, 0);
		}

		//平行光源の設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		//インデックスの描画
		dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);//IAVを設定
		//描画
		if (isDisplayIndex) {
			dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
		}

		//モデルの描画
		DrawModel(dxCommon->GetCommandList(), modelResource, isDisplayModel);
		//モデル2の描画
		DrawModel(dxCommon->GetCommandList(), model2Resource, isDisplayModel2);
		//モデル3の描画
		DrawModel(dxCommon->GetCommandList(), model3Resource, isDisplayModel3);
		//モデル4の描画
		DrawModel(dxCommon->GetCommandList(), model4Resource, isDisplayModel4);
		//モデル5の描画
		DrawModel(dxCommon->GetCommandList(), model5Resource, isDisplayModel5);
		//モデル6の描画
		DrawModel(dxCommon->GetCommandList(), model6Resource, isDisplayModel6);
		//モデル7の描画
		DrawModel(dxCommon->GetCommandList(), model7Resource, isDisplayModel7);
		//モデル8の描画
		DrawModel(dxCommon->GetCommandList(), model8Resource, isDisplayModel8);


		//ImGuiの描画
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList().Get());

		//描画後処理
		dxCommon->PostDraw();
	}


	//ImGuiの終了処理。
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	/////解放処理/////

	//xAudio2
	xAudio2.Reset();
	//音声データ
	SoundUnload(&soundData1);

	delete input;

	//WindowsAPIの終了処理
	winApp->Finalize();
	delete winApp;
	delete dxCommon;
	delete leakChecker;

	return 0;
}
