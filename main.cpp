#include <fstream>
#include <xaudio2.h>
#include "DirectXCommon.h"
#include "WinApp.h"
#include "Input.h"
#include "D3DResourceLeakChecker.h"
#include "Logger.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Function.h"

#pragma comment(lib,"xaudio2.lib")

//定数
const int kTriangleVertexNum = 3;
const int kTriangleNum = 2;

const uint32_t kSubdivision = 20;

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

////mtlファイルを読む関数
//MaterialData LoadMaterialTemplateFile(const std::string& directryPath, const std::string& filename, const std::string& materialName) {
//	//1.中で必要となる変数の宣言
//	MaterialData materialData;
//	std::string line;
//	bool isLoad = false;
//	//2.ファイルを開く
//	std::ifstream file(directryPath + "/" + filename);
//	assert(file.is_open());
//	//3.実際にファイルを読み、MaterialDAtaを構築していく
//	while (std::getline(file, line)) {
//		std::string identifier;
//		std::istringstream s(line);
//		s >> identifier;
//		//identifierに応じた処理
//		if (identifier == "newmtl") {
//			std::string mtlNumber;
//			s >> mtlNumber;
//			//読み込みたいマテリアルの名前が一致してたら読み込み、そうじゃないなら読み込まない
//			if (mtlNumber == materialName) {
//				isLoad = true;
//			}
//			else {
//				isLoad = false;
//			}
//		}
//		if (isLoad) {
//			if (identifier == "map_Kd") {
//				std::string textureFilename;
//				s >> textureFilename;
//				//連結してファイルパスにする
//				materialData.textureFilePath = directryPath + "/" + textureFilename;
//			}
//			else if (identifier == "Kd") {
//				Vector4 color;
//				s >> color.x >> color.y >> color.z;
//				color.w = 1.0f;
//				materialData.colorData = color;
//			}
//		}
//	}
//	//4.MaterialDataを返す
//	return materialData;
//}

////objファイル読む関数
//std::vector<ModelData> LoadObjFIle(const std::string& directoryPath, const std::string& filename) {
//	//1.中で必要となる変数の宣言
//	std::vector<ModelData> modelData;
//	std::vector<Vector4> positions;
//	std::vector<Vector3> normals;
//	std::vector<Vector2> texcoords;
//	std::string line;
//	std::string line2;
//	bool isGetO = false;
//	bool isGetVt = false;
//	size_t index = -1;
//	//2.ファイルを開く
//	std::ifstream file(directoryPath + "/" + filename);
//	assert(file.is_open());
//	//3.実際にファイルを読み、ModelDataを構築していく
//	while (std::getline(file, line)) {
//		std::string identifier;
//		std::istringstream s(line);
//		s >> identifier;//←先頭の識別子を読む
//		//identifierに応じた処理
//		if (identifier == "o") {
//			index++;
//			modelData.resize(index + 1);
//			isGetO = true;
//		}
//		else if (identifier == "v") {
//			Vector4 position;
//			s >> position.x >> position.y >> position.z;
//			position.x *= -1.0f;
//			position.w = 1.0f;
//			positions.push_back(position);
//		}
//		else if (identifier == "vt") {
//			Vector2 texcoord;
//			s >> texcoord.x >> texcoord.y;
//			texcoord.y = 1.0f - texcoord.y;
//			texcoords.push_back(texcoord);
//			isGetVt = true;
//		}
//		else if (identifier == "vn") {
//			Vector3 normal;
//			s >> normal.x >> normal.y >> normal.z;
//			normal.x *= -1.0f;
//			normals.push_back(normal);
//		}
//		else if (identifier == "f") {
//			VertexData triangle[3];
//			//面は三角形限定。その他は未対応
//			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
//				std::string vertexDefinition;
//				s >> vertexDefinition;
//				//頂点の要素へのstrは「位置/UV/法線」で格納されているので、分解してstrを取得する
//				std::istringstream v(vertexDefinition);
//				uint32_t elementIndices[3];
//				for (int32_t element = 0; element < 3; ++element) {
//					std::string str;
//					std::getline(v, str, '/');
//					if (str.size() == 0) {
//						//データがない場合、数値0を仮に入れておく
//						str = "0";
//					}
//					elementIndices[element] = std::stoi(str);
//				}
//				//要素へのstrから、実際の要素の値を取得して、頂点を構築する
//				Vector4 position;
//				Vector2 texcoord;
//				Vector3 normal;
//				if (isGetVt) {
//					position = positions[elementIndices[0] - 1];
//					texcoord = texcoords[elementIndices[1] - 1];
//					normal = normals[elementIndices[2] - 1];
//				}
//				else {
//					position = positions[elementIndices[0] - 1];
//					texcoord = { 0,0 };//texcoord(0,0)を代入
//					normal = normals[elementIndices[2] - 1];
//				}
//				triangle[faceVertex] = { position,texcoord,normal };
//
//			}
//			//頂点を逆順に登録することで、周り順を逆にする
//			modelData.at(index).vertices.push_back(triangle[2]);
//			modelData.at(index).vertices.push_back(triangle[1]);
//			modelData.at(index).vertices.push_back(triangle[0]);
//		}
//		else if (identifier == "usemtl") {
//			std::string materialName;
//			s >> materialName;
//			//例外処理(objファイルにoがない場合)
//			if (!isGetO) {
//				index++;
//				modelData.resize(index + 1);
//			}
//			//各オブジェクト毎のマテリアル名を記憶させる
//			modelData.at(index).materialName = materialName;
//		}
//	}
//	//ファイルの読み直し
//	std::ifstream file2(directoryPath + "/" + filename);
//	assert(file.is_open());
//	//mtlファイルを開いてマテリアル情報を得る(materialNameを基に)
//	while (std::getline(file2, line2)) {
//		std::string identifier;
//		std::istringstream s(line2);
//		s >> identifier;//←先頭の識別子を読む
//		//identifierに応じた処理
//		if (identifier == "mtllib") {
//			//materialTemplateLibraryファイルの名前を取得する
//			std::string materialFilename;
//			s >> materialFilename;
//			for (size_t index = 0; index < modelData.size(); index++) {
//				//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を探す。
//				modelData.at(index).material = LoadMaterialTemplateFile(directoryPath, materialFilename, modelData.at(index).materialName);
//			}
//			//読んだら抜ける
//			break;
//		}
//	}
//	//4.ModelDataを返す
//	return modelData;
//}

//void DrawModel(
//	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList,
//	const ModelResource modelResource,
//	const bool isDisplayModel
//) {
//	for (size_t index = 0; index < modelResource.modelData.size(); index++) {
//		//モデルの描画
//		commandList->IASetVertexBuffers(0, 1, &modelResource.vertexBufferView.at(index));
//		//マテリアルCBufferの場所を設定
//		commandList->SetGraphicsRootConstantBufferView(0, modelResource.materialResource.at(index)->GetGPUVirtualAddress());
//		//モデルにテクスチャがない場合、スキップ
//		if (modelResource.modelData.at(index).material.textureFilePath.size() != 0) {
//			//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]でテクスチャの設定をしているため。
//			commandList->SetGraphicsRootDescriptorTable(2, modelResource.textureSrvHandleGPU.at(index));
//		}
//		//wvp用のCBufferの場所を指定
//		commandList->SetGraphicsRootConstantBufferView(1, modelResource.wvpResource->GetGPUVirtualAddress());
//		//モデルの描画
//		if (isDisplayModel) {
//			commandList->DrawInstanced(UINT(modelResource.modelData.at(index).vertices.size()), 1, 0, 0);
//		}
//	}
//}

//void MakeModelResource(
//	const char* resourceFileName,
//	const char* objFileName,
//	DirectXCommon* dxCommon,
//	ModelResource& modelResource
//) {
//	modelResource.modelData = LoadObjFIle(resourceFileName, objFileName);
//	const size_t kModelNum = modelResource.modelData.size();
//	//std::vector型の要素数を確定
//	modelResource.vertexResource.resize(kModelNum);
//	modelResource.vertexBufferView.resize(kModelNum);
//	modelResource.vertexData.resize(kModelNum);
//	modelResource.materialResource.resize(kModelNum);
//	modelResource.materialData.resize(kModelNum);
//	modelResource.textureResorce.resize(kModelNum);
//	modelResource.textureSrvHandleCPU.resize(kModelNum);
//	modelResource.textureSrvHandleGPU.resize(kModelNum);
//	modelResource.uvTransform.resize(kModelNum);
//	for (size_t index = 0; index < kModelNum; index++) {
//		//頂点用リソースを作る
//		modelResource.vertexResource.at(index) = dxCommon->CreateBufferResource(sizeof(VertexData) * modelResource.modelData.at(index).vertices.size());
//		//頂点バッファービューを作成
//		modelResource.vertexBufferView.at(index).BufferLocation = modelResource.vertexResource.at(index)->GetGPUVirtualAddress();
//		modelResource.vertexBufferView.at(index).SizeInBytes = UINT(sizeof(VertexData) * modelResource.modelData.at(index).vertices.size());
//		modelResource.vertexBufferView.at(index).StrideInBytes = sizeof(VertexData);
//		//頂点用リソースにデータを書き込む
//		modelResource.vertexResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&modelResource.vertexData.at(index)));
//		std::memcpy(modelResource.vertexData.at(index), modelResource.modelData.at(index).vertices.data(), sizeof(VertexData) * modelResource.modelData.at(index).vertices.size());
//		//マテリアル用のリソースを作る。
//		modelResource.materialResource.at(index) = dxCommon->CreateBufferResource(sizeof(Material));
//		//書き込むためのアドレスを取得
//		modelResource.materialResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&modelResource.materialData.at(index)));
//		//白を書き込んでおく
//		modelResource.materialData.at(index)->color = modelResource.modelData.at(index).material.colorData;
//		//ライティング
//		modelResource.materialData.at(index)->lightingKind = HalfLambert;
//		//uvTransform
//		modelResource.materialData.at(index)->uvTransform = MakeIdentity4x4();
//		//テクスチャを持っているか
//		bool isTexture = true;
//		if (modelResource.modelData.at(index).material.textureFilePath.size() == 0) {
//			//テクスチャファイルパスに書き込まれていない→テクスチャがない
//			isTexture = false;
//		}
//		modelResource.materialData.at(index)->isTexture = isTexture;
//		//UVトランスフォーム
//		modelResource.uvTransform.at(index) = {
//			{1.0f,1.0f,1.0f},
//			{0.0f,0.0f,0.0f},
//			{0.0f,0.0f,0.0f}
//		};
//	}
//	//WVP用のリソースを作る。
//	modelResource.wvpResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
//	//データを書き込む
//	modelResource.wvpData = nullptr;
//	//書き込むためのアドレスを取得
//	modelResource.wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&modelResource.wvpData));
//	//単位行列を書き込んでおく
//	modelResource.wvpData->WVP = MakeIdentity4x4();
//	modelResource.wvpData->World = MakeIdentity4x4();
//	//トランスフォーム
//	modelResource.transform = {
//		{1.0f,1.0f,1.0f},
//		{0.0f,0.0f,0.0f},
//		{0.0f,0.0f,0.0f}
//	};
//}

////テクスチャ設定用関数
//void SetTexture(
//	ModelResource& modelResource,
//	DirectXCommon* dxCommon,
//	uint32_t& site
//) {
//	for (size_t i = 0; i < modelResource.modelData.size(); i++) {
//		//もし、モデルのテクスチャがないなら、この処理はしない
//		if (modelResource.modelData.at(i).material.textureFilePath.size() == 0) {
//			break;
//		}
//		//モデル用のTextureを読んで転送する
//		DirectX::ScratchImage mipImagesModel = DirectXCommon::LoadTexture(modelResource.modelData.at(i).material.textureFilePath);
//		const DirectX::TexMetadata& metadataModel = mipImagesModel.GetMetadata();
//		modelResource.textureResorce.at(i) = dxCommon->CreateTextureResource(metadataModel);
//		dxCommon->UploadTextureData(modelResource.textureResorce.at(i), mipImagesModel);
//		//metadataをもとにSRVの設定
//		D3D12_SHADER_RESOURCE_VIEW_DESC srvDescModel{};
//		srvDescModel.Format = metadataModel.format;
//		srvDescModel.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//		srvDescModel.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//		srvDescModel.Texture2D.MipLevels = UINT(metadataModel.mipLevels);
//		//DescriptorHeapの場所移動
//		site++;
//		//SRVを作成するDescriptorHeapの場所を決める
//		modelResource.textureSrvHandleCPU.at(i) = dxCommon->GetSRVCPUDescriptorHandle(site);
//		modelResource.textureSrvHandleGPU.at(i) = dxCommon->GetSRVGPUDescriptorHandle(site);
//		//SRVの作成
//		dxCommon->GetDevice()->CreateShaderResourceView(modelResource.textureResorce.at(i).Get(), &srvDescModel, modelResource.textureSrvHandleCPU.at(i));
//	}
//}

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
	D3DResourceLeakChecker leakChecker;

	//ウィンドウ
	WinApp* winApp = nullptr;
	//WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	//DorectX12
	DirectXCommon* dxCommon = nullptr;
	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//テクスチャマネージャ
	TextureManager::GetInstance()->Initialize(dxCommon);

	//インプット
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp);

	//スプライト共通部
	SpriteCommon* spriteCommon = nullptr;
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);



	HRESULT hr;


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



	///////////////////////////Sphere用の頂点リソースを作る/////////////////////////////////////////////////////////////////////////////////
	//Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere = dxCommon->CreateBufferResource(sizeof(VertexData) * (kSubdivision * kSubdivision * 6));
	////頂点バッファビューを作成する
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	////リソースの先端のアドレスから使う
	//vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	////使用するリソースのサイズは頂点6つ分のサイズ
	//vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * (kSubdivision * kSubdivision * 6);
	////1頂点あたりのサイズ
	//vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);
	////頂点リソースにデータを書き込む
	//VertexData* vertexDataSphere = nullptr;
	////書き込むためのアドレスを取得
	//vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));
	/////頂点の座標を決めていく
	////経度分割1つ分の角度
	//const float kLonEvery = pi * 2.0f / float(kSubdivision);
	////緯度分割1つ分の角度
	//const float kLatEvery = pi / float(kSubdivision);
	////球の情報
	//Sphere sphere;
	//sphere.center = { 0.0f,0.0f,0.0f };
	//sphere.radius = 1.0f;
	////緯度の方向に分割
	//for (UINT latIndex = 0; latIndex < kSubdivision; ++latIndex) {
	//	float lat = -pi / 2.0f + kLatEvery * latIndex;
	//	//緯度の方向に分割
	//	for (UINT lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
	//		uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
	//		float lon = lonIndex * kLonEvery;
	//		///三角形一枚目
	//		//頂点にデータを入れる
	//		vertexDataSphere[start].position.x = sphere.center.x + sphere.radius * cos(lat) * cos(lon);
	//		vertexDataSphere[start].position.y = sphere.center.y + sphere.radius * sin(lat);
	//		vertexDataSphere[start].position.z = sphere.center.z + sphere.radius * cos(lat) * sin(lon);
	//		vertexDataSphere[start].position.w = 1.0f;
	//		vertexDataSphere[start].texcoord.x = float(lonIndex) / float(kSubdivision);
	//		vertexDataSphere[start].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
	//		//頂点にデータを入れる
	//		vertexDataSphere[start + 1].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon);
	//		vertexDataSphere[start + 1].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
	//		vertexDataSphere[start + 1].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon);
	//		vertexDataSphere[start + 1].position.w = 1.0f;
	//		vertexDataSphere[start + 1].texcoord.x = float(lonIndex) / float(kSubdivision);
	//		vertexDataSphere[start + 1].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
	//		//頂点にデータを入れる
	//		vertexDataSphere[start + 2].position.x = sphere.center.x + sphere.radius * cos(lat) * cos(lon + kLonEvery);
	//		vertexDataSphere[start + 2].position.y = sphere.center.y + sphere.radius * sin(lat);
	//		vertexDataSphere[start + 2].position.z = sphere.center.z + sphere.radius * cos(lat) * sin(lon + kLonEvery);
	//		vertexDataSphere[start + 2].position.w = 1.0f;
	//		vertexDataSphere[start + 2].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
	//		vertexDataSphere[start + 2].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
	//		///三角形二枚目
	//		//頂点にデータを入れる
	//		vertexDataSphere[start + 3].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon);
	//		vertexDataSphere[start + 3].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
	//		vertexDataSphere[start + 3].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon);
	//		vertexDataSphere[start + 3].position.w = 1.0f;
	//		vertexDataSphere[start + 3].texcoord.x = float(lonIndex) / float(kSubdivision);
	//		vertexDataSphere[start + 3].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
	//		//頂点にデータを入れる
	//		vertexDataSphere[start + 4].position.x = sphere.center.x + sphere.radius * cos(lat + kLatEvery) * cos(lon + kLonEvery);
	//		vertexDataSphere[start + 4].position.y = sphere.center.y + sphere.radius * sin(lat + kLatEvery);
	//		vertexDataSphere[start + 4].position.z = sphere.center.z + sphere.radius * cos(lat + kLatEvery) * sin(lon + kLonEvery);
	//		vertexDataSphere[start + 4].position.w = 1.0f;
	//		vertexDataSphere[start + 4].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
	//		vertexDataSphere[start + 4].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
	//		//頂点にデータを入れる
	//		vertexDataSphere[start + 5].position.x = sphere.center.x + sphere.radius * cos(lat) * cos(lon + kLonEvery);
	//		vertexDataSphere[start + 5].position.y = sphere.center.y + sphere.radius * sin(lat);
	//		vertexDataSphere[start + 5].position.z = sphere.center.z + sphere.radius * cos(lat) * sin(lon + kLonEvery);
	//		vertexDataSphere[start + 5].position.w = 1.0f;
	//		vertexDataSphere[start + 5].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
	//		vertexDataSphere[start + 5].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);

	//		//法線情報の入力
	//		for (uint32_t i = 0; i < 6; i++) {
	//			vertexDataSphere[start + i].normal.x = vertexDataSphere[start + i].position.x;
	//			vertexDataSphere[start + i].normal.y = vertexDataSphere[start + i].position.y;
	//			vertexDataSphere[start + i].normal.z = vertexDataSphere[start + i].position.z;
	//		}

	//		//メモ//
	//		//三角形描画時には頂点を半時計周りの順番で設定する。
	//		//時計回りにすると表裏が逆になってしまう。
	//	}
	//}
	////マテリアル用のリソースを作る。
	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSphere = dxCommon->CreateBufferResource(sizeof(Material));
	////データを書き込む
	//Material* materialDataSphere = nullptr;
	////書き込むためのアドレスを取得
	//materialResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
	////白を書き込んでおく
	//materialDataSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	////ライティングオン
	//materialDataSphere->lightingKind = HalfLambert;
	////uvTransform
	//materialDataSphere->uvTransform = MakeIdentity4x4();
	////テクスチャを持っているか
	//materialDataSphere->isTexture = true;
	////WVP用のリソースを作る。
	//Microsoft::WRL::ComPtr<ID3D12Resource> wvpResourceSphere = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	////データを書き込む
	//TransformationMatrix* wvpDataSphere = nullptr;
	////書き込むためのアドレスを取得
	//wvpResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSphere));
	////単位行列を書き込んでおく
	//wvpDataSphere->WVP = MakeIdentity4x4();
	//wvpDataSphere->World = MakeIdentity4x4();
	////トランスフォーム
	//Transform transformSphere = {
	//	{1.0f,1.0f,1.0f},
	//	{0.0f,0.0f,0.0f},
	//	{0.0f,0.0f,0.0f}
	//};
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

	//////////////////Xaudio2の設定/////////////////////////////////////
	//必要な変数の宣言
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;
	///////////////////////////////////////////////////////////////////


	//初期化
#pragma region 最初のシーン初期化
	Sprite* sprite = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	sprite->Initialize(spriteCommon, "Resources/uvChecker.png");
	
	Sprite* sprite2 = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	sprite2->Initialize(spriteCommon, "Resources/monsterBall.png");
	sprite2->SetPosition({ 0.0f,360.0f });

#pragma endregion 最初のシーンの終了

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

		//スプライトの更新
		sprite->Update();
		sprite2->Update();


		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える

		ImGui::Begin("Settings");


		ImGui::End();

		/////レンダリングパイプライン/////

		//ImGuiの内部コマンドを生成する
		ImGui::Render();

		//////////////////////
		///描画処理
		//////////////////////

		//描画前処理
		dxCommon->PreDraw();

		//スプライトの描画設定
		spriteCommon->SettingCommonDrawing();

		//スプライト描画
		sprite->Draw();
		sprite2->Draw();

		//平行光源の設定
		dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		//ImGuiの描画
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

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

	delete sprite2;
	delete sprite;
	delete spriteCommon;
	delete input;
	TextureManager::GetInstance()->Finalize();
	delete dxCommon;
	//WindowsAPIの終了処理
	winApp->Finalize();
	delete winApp;

	return 0;
}
