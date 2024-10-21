#include "Model.h"
#include "DirectXCommon.h"
#include "ModelCommon.h"
#include "fstream"
#include "sstream"
#include "Function.h"
#include "TextureManager.h"
#include "WinApp.h"
#include <cassert>

void Model::Initialize(ModelCommon* modelCommon)
{
	//引数で受け取ってメンバ変数に記録する
	modelCommon_ = modelCommon;

	//一旦planeでリソースを作る
	MakeModelResource("Resources", "plane.obj");
	SettingTexture();

	//平行光源用リソースを作る
	directionalLightResource = modelCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::DirectionalLight));
	//リソースにデータをセット
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//平行光源用データ
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

}

void Model::Update()
{
	modelResource_.transform.rotate.y += 0.03f;

	//レンダリングパイプライン
	Matrix4x4 worldMatrix = MakeAffineMatrix(modelResource_.transform.scale, modelResource_.transform.rotate, modelResource_.transform.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	modelResource_.wvpData->WVP = worldViewProjectionMatrix;
	modelResource_.wvpData->World = worldMatrix;

}

void Model::Draw()
{
	for (size_t index = 0; index < modelResource_.modelData.size(); index++) {
		//頂点バッファービューを設定
		modelCommon_->GetDirectXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &modelResource_.vertexBufferView.at(index));

		//マテリアルCBufferの場所を設定
		modelCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, modelResource_.materialResource.at(index)->GetGPUVirtualAddress());
		//座標変換行列CBufferの場所を設定
		modelCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1,modelResource_.wvpResource->GetGPUVirtualAddress());
		//モデルにテクスチャがない場合、スキップ
		if (modelResource_.modelData.at(index).material.textureFilePath.size() != 0) {
			//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]でテクスチャの設定をしているため。
			modelCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelResource_.modelData.at(index).material.textureIndex));
		}

		//平行光源の設定
		modelCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		//描画
		modelCommon_->GetDirectXCommon()->GetCommandList()->DrawInstanced(UINT(modelResource_.modelData.at(index).vertices.size()), 1, 0, 0);
	}
}

Model::Struct::MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName, const std::string& materialName)
{
	//1.中で必要となる変数の宣言
	Struct::MaterialData materialData;
	std::string line;
	bool isLoad = false;
	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + fileName);
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
				materialData.textureFilePath = directoryPath + "/" + textureFilename;
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

std::vector<Model::Struct::ModelData> Model::LoadObjFile(const std::string& directoryPath, const std::string& fileName)
{
	//1.中で必要となる変数の宣言
	std::vector<Struct::ModelData> modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;
	std::string line2;
	bool isGetO = false;
	bool isGetVt = false;
	size_t index = -1;
	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + fileName);
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
			Struct::VertexData triangle[3];
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
	std::ifstream file2(directoryPath + "/" + fileName);
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

void Model::MakeModelResource(const std::string& resourceFileName, const std::string& objFileName)
{
	modelResource_.modelData = LoadObjFile(resourceFileName, objFileName);
	const size_t kModelNum = modelResource_.modelData.size();
	//std::vector型の要素数を確定
	modelResource_.vertexResource.resize(kModelNum);
	modelResource_.vertexBufferView.resize(kModelNum);
	modelResource_.vertexData.resize(kModelNum);
	modelResource_.materialResource.resize(kModelNum);
	modelResource_.materialData.resize(kModelNum);
	modelResource_.textureResorce.resize(kModelNum);
	modelResource_.textureSrvHandleCPU.resize(kModelNum);
	modelResource_.textureSrvHandleGPU.resize(kModelNum);
	modelResource_.uvTransform.resize(kModelNum);
	for (size_t index = 0; index < kModelNum; index++) {
		//頂点用リソースを作る
		modelResource_.vertexResource.at(index) = modelCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::VertexData) * modelResource_.modelData.at(index).vertices.size());
		//マテリアル用のリソースを作る。
		modelResource_.materialResource.at(index) = modelCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::Material));
		//WVP用のリソースを作る
		modelResource_.wvpResource = modelCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::TransformationMatrix));
		//頂点バッファービューを作成
		modelResource_.vertexBufferView.at(index).BufferLocation = modelResource_.vertexResource.at(index)->GetGPUVirtualAddress();
		modelResource_.vertexBufferView.at(index).SizeInBytes = UINT(sizeof(Struct::VertexData) * modelResource_.modelData.at(index).vertices.size());
		modelResource_.vertexBufferView.at(index).StrideInBytes = sizeof(Struct::VertexData);
		//リソースにデータを書き込む
		modelResource_.vertexResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&modelResource_.vertexData.at(index)));
		std::memcpy(modelResource_.vertexData.at(index), modelResource_.modelData.at(index).vertices.data(), sizeof(Struct::VertexData) * modelResource_.modelData.at(index).vertices.size());
		modelResource_.materialResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&modelResource_.materialData.at(index)));
		modelResource_.wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&modelResource_.wvpData));
		//白を書き込んでおく
		modelResource_.materialData.at(index)->color = modelResource_.modelData.at(index).material.colorData;
		//ライティング
		modelResource_.materialData.at(index)->lightingKind = HalfLambert;
		//Transform
		modelResource_.wvpData->WVP = MakeIdentity4x4();
		modelResource_.wvpData->World = MakeIdentity4x4();
		//uvTransform
		modelResource_.materialData.at(index)->uvTransform = MakeIdentity4x4();
		//テクスチャを持っているか
		bool isTexture = true;
		if (modelResource_.modelData.at(index).material.textureFilePath.size() == 0) {
			//テクスチャファイルパスに書き込まれていない→テクスチャがない
			isTexture = false;
		}
		modelResource_.materialData.at(index)->isTexture = isTexture;
		//トランスフォーム
		modelResource_.transform = {
			{1.0f,1.0f,1.0f},
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f}
		};
		//UVトランスフォーム
		modelResource_.uvTransform.at(index) = {
			{1.0f,1.0f,1.0f},
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f}
		};
	}
}

void Model::SettingTexture()
{
	for (size_t index = 0; index < modelResource_.modelData.size(); index++) {
		//.objの参照しているテクスチャファイル読み込み
		TextureManager::GetInstance()->LoadTexture(modelResource_.modelData.at(index).material.textureFilePath);
		//読み込んだテクスチャの番号を取得
		modelResource_.modelData.at(index).material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelResource_.modelData.at(index).material.textureFilePath);
	}
}
