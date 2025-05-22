#include "Model.h"
#include "DirectXCommon.h"
#include "MainRender.h"
#include "TextureManager.h"
#include <fstream>
#include <sstream>
#include <filesystem>

void Model::Initialize(const std::string& filename, ModelFormat format, std::string directorypath) {
	//ディレクトリパス
	directoryPath_ = directorypath;
	//モデルデータの形式
	mf_ = format;
	switch (mf_) {
	case OBJ:
		format_ = ".obj";
		break;
	case GLTF:
		format_ = ".gltf";
		break;
	default:
		break;
	}
	//ファイル名
	fileName_ = filename;

	//モデルリソースの初期設定
	modelResource_ = MakeModelResource();
	//テクスチャの設定
	SettingTexture();
}

void Model::Draw(uint32_t materialRootParameterIndex, uint32_t textureRootParameterIndex, uint32_t instancingNum, int32_t textureHandle) {
	for (size_t index = 0; index < modelResource_.modelData.size(); index++) {
		//頂点バッファービューを設定
		MainRender::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &modelResource_.vertexBufferView.at(index));
		//インデックスバッファビューを設定
		MainRender::GetInstance()->GetCommandList()->IASetIndexBuffer(&modelResource_.indexBufferView.at(index));
		//マテリアルCBufferの場所を設定
		if (color_) {
			modelResource_.materialData.at(index)->color = *color_;
		}
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(materialRootParameterIndex, modelResource_.materialResource.at(index)->GetGPUVirtualAddress());
		//テクスチャが外部から設定されている場合
		if (textureHandle != -1) {
			//SRVのDescriptorTableの先頭を設定。
			MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(textureRootParameterIndex, TextureManager::GetInstance()->GetSrvHandleGPU(textureHandle));
		}
		else {
			//モデルにテクスチャがない場合、スキップ
			if (modelResource_.modelData.at(index).material.textureFilePath.size() != 0) {
				//SRVのDescriptorTableの先頭を設定。
				MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(textureRootParameterIndex, TextureManager::GetInstance()->GetSrvHandleGPU(modelResource_.modelData.at(index).material.textureHandle));
			}
		}

		//描画
		MainRender::GetInstance()->GetCommandList()->DrawIndexedInstanced(UINT(modelResource_.modelData.at(index).indices.size()), instancingNum, 0, 0, 0);
	}
}

std::vector<Model::ModelData> Model::LoadModelFile() {
	// 必要な変数の宣言
	std::vector<ModelData> modelData;

	// Assimpでシーンを読み込み
	Assimp::Importer importer;
	std::string filePath = directoryPath_ + fileName_ + "/" + fileName_ + format_;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes()); // メッシュがない場合はエラー

	// モデルデータのサイズ設定
	modelData.resize(scene->mNumMeshes);

	// メッシュを解析してモデルデータに格納
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());      // 法線がない場合はエラー
		assert(mesh->HasTextureCoords(0)); // TexCoordがない場合はエラー

		// モデルデータを準備
		ModelData model;
		model.vertices.clear();

		// メッシュが使用するマテリアルのインデックスを取得
		uint32_t materialIndex = mesh->mMaterialIndex;
		aiMaterial* material = scene->mMaterials[materialIndex];

		// マテリアル名を取得
		aiString materialName;
		if (material->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS) {
			model.material.materialName = materialName.C_Str();
		}

		// Kd (拡散色) を取得
		aiColor3D kd(0.8f, 0.8f, 0.8f); // デフォルト値
		if (material->Get(AI_MATKEY_COLOR_DIFFUSE, kd) == AI_SUCCESS) {
			model.material.colorData = { kd.r, kd.g, kd.b, 1.0f };
		}
		else {
			model.material.colorData = { 0.8f, 0.8f, 0.8f, 1.0f };
		}

		// テクスチャパスを取得
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath) == AI_SUCCESS) {
				std::string fullPath = textureFilePath.C_Str();
				model.material.textureFilePath = std::filesystem::path(fullPath).filename().string();
			}
		}
		else {
			model.material.textureFilePath = ""; // テクスチャがない場合は空文字列
		}

		//メッシュ内の頂点データを解析
		model.vertices.resize(mesh->mNumVertices);
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

			model.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
			model.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
			model.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
		}

		// メッシュ内のフェイスを解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); // 三角形のみサポート
			//フェイス内のインデックスデータの解析
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				//インデックス
				model.indices.push_back(vertexIndex);
			}
		}

		// 構築したモデルデータにルートノードを設定
		switch (mf_) {
		case OBJ:
			break;
		case GLTF:
			model.rootNode = ReadNode(scene->mRootNode);
			break;
		default:
			break;
		}

		// 構築したモデルデータを格納
		modelData[meshIndex] = model;
	}

	//ModelDataを返す
	return modelData;
}

Model::Node Model::ReadNode(aiNode* node) {
	// 新しいNodeオブジェクトを作成
	Node currentNode;

	// ノード名を設定
	currentNode.name = node->mName.C_Str();

	// ローカルマトリックスを取得して設定
	aiMatrix4x4 aiMatrix = node->mTransformation;
	currentNode.localMatrix = {
		aiMatrix.a1, aiMatrix.a2, aiMatrix.a3, aiMatrix.a4,
		aiMatrix.b1, aiMatrix.b2, aiMatrix.b3, aiMatrix.b4,
		aiMatrix.c1, aiMatrix.c2, aiMatrix.c3, aiMatrix.c4,
		aiMatrix.d1, aiMatrix.d2, aiMatrix.d3, aiMatrix.d4,
	};

	// 子ノードを再帰的に処理
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		currentNode.children.push_back(ReadNode(node->mChildren[i]));
	}

	return currentNode;
}

Model::ModelResource Model::MakeModelResource() {
	//モデルリソース
	ModelResource modelResource_;

	modelResource_.modelData = LoadModelFile();
	modelNum_ = modelResource_.modelData.size();
	//std::vector型の要素数を確定
	modelResource_.vertexResource.resize(modelNum_);
	modelResource_.vertexBufferView.resize(modelNum_);
	modelResource_.vertexData.resize(modelNum_);
	modelResource_.indexResource.resize(modelNum_);
	modelResource_.indexBufferView.resize(modelNum_);
	modelResource_.indexData.resize(modelNum_);
	modelResource_.materialResource.resize(modelNum_);
	modelResource_.materialData.resize(modelNum_);
	modelResource_.textureResorce.resize(modelNum_);
	modelResource_.textureSrvHandleCPU.resize(modelNum_);
	modelResource_.textureSrvHandleGPU.resize(modelNum_);
	modelResource_.uvTransform.resize(modelNum_);
	for (size_t index = 0; index < modelNum_; index++) {
		//頂点用リソースを作る
		modelResource_.vertexResource.at(index) = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * modelResource_.modelData.at(index).vertices.size());
		//インデックス描画用のリソースを作る
		modelResource_.indexResource.at(index) = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * modelResource_.modelData.at(index).indices.size());
		//マテリアル用のリソースを作る。
		modelResource_.materialResource.at(index) = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
		//頂点バッファービューを作成
		modelResource_.vertexBufferView.at(index).BufferLocation = modelResource_.vertexResource.at(index)->GetGPUVirtualAddress();
		modelResource_.vertexBufferView.at(index).SizeInBytes = UINT(sizeof(VertexData) * modelResource_.modelData.at(index).vertices.size());
		modelResource_.vertexBufferView.at(index).StrideInBytes = sizeof(VertexData);
		//インデックスバッファビューを作成
		modelResource_.indexBufferView.at(index).BufferLocation = modelResource_.indexResource.at(index)->GetGPUVirtualAddress();
		modelResource_.indexBufferView.at(index).SizeInBytes = UINT(sizeof(uint32_t) * modelResource_.modelData.at(index).indices.size());
		modelResource_.indexBufferView.at(index).Format = DXGI_FORMAT_R32_UINT;
		//リソースにデータを書き込む
		modelResource_.vertexResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&modelResource_.vertexData.at(index)));
		std::memcpy(modelResource_.vertexData.at(index), modelResource_.modelData.at(index).vertices.data(), sizeof(VertexData) * modelResource_.modelData.at(index).vertices.size());
		modelResource_.indexResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&modelResource_.indexData.at(index)));
		std::memcpy(modelResource_.indexData.at(index), modelResource_.modelData.at(index).indices.data(), sizeof(uint32_t) * modelResource_.modelData.at(index).indices.size());
		modelResource_.materialResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&modelResource_.materialData.at(index)));
		//白を書き込んでおく
		modelResource_.materialData.at(index)->color = modelResource_.modelData.at(index).material.colorData;
		//uvTransform
		modelResource_.materialData.at(index)->uvTransform = MyMath::MakeIdentity4x4();
		//テクスチャを持っているか
		bool isTexture = true;
		if (modelResource_.modelData.at(index).material.textureFilePath.size() == 0) {
			//テクスチャファイルパスに書き込まれていない→テクスチャがない
			isTexture = false;
		}
		modelResource_.materialData.at(index)->isTexture = isTexture;
		modelResource_.materialData.at(index)->shininess = 20.0f;
		//UVトランスフォーム
		modelResource_.uvTransform.at(index) = {
			{1.0f,1.0f,1.0f},
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f}
		};
	}

	return modelResource_;
}

void Model::SettingTexture() {
	for (size_t index = 0; index < modelResource_.modelData.size(); index++) {
		//.objの参照しているテクスチャファイル読み込み(TextureManagerはResources/をカットできるので)
		modelResource_.modelData.at(index).material.textureHandle = TextureManager::GetInstance()->LoadTexture("../models/" + fileName_ + "/" + modelResource_.modelData.at(index).material.textureFilePath);
	}
}