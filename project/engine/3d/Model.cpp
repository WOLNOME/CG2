#include "Model.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include "DirectXCommon.h"
#include "MainRender.h"
#include "Object3d.h"
#include "TextureManager.h"



void Model::Initialize(const std::string& filename, ModelFormat format, std::string directorypath)
{
	//ディレクトリパス
	directoryPath_ = directorypath;
	//モデルデータの形式
	mf_ = format;
	switch (mf_)
	{
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

	//デバッグ用のラインのサイズをjointの数と揃える
	if (isSkeleton_) {
		lines_.resize(skeleton_.joints.size() + 1);
		for (auto& line : lines_) {
			line = std::make_unique<LineDrawer>();
			line->Initialize();
		}
	}

	//テクスチャの設定
	SettingTexture();
}

void Model::Update()
{
	//アニメーションの更新と骨への適用
	if (isAnimation_ && isSkeleton_) {
		animationTime_ += kDeltaTime;
		animationTime_ = std::fmod(animationTime_, animation_.duration);
		ApplyAnimation(skeleton_, animation_, animationTime_);
		UpdateJoints(skeleton_);
	}
}

void Model::UpdateJoints(Skeleton& skeleton)
{
	//全てのJointを更新
	for (Joint& joint : skeleton.joints) {
		joint.localMatrix = MyMath::MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
		}
		else {
			//親がいない(root)のでlocalMatrix=skeletonSpaceMatrixである
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}

}

void Model::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime)
{
	for (Joint& joint : skeleton.joints) {
		//対象のJointのAnimationがあれば、値の適用を行う。下記のif文は初期化付きif文。
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.translate = CalculateValue(rootNodeAnimation.translate.keyframes, animationTime);
			joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime);
			joint.transform.scale = CalculateValue(rootNodeAnimation.scale.keyframes, animationTime);
		}
	}
}

void Model::Draw(uint32_t materialRootParameterIndex, uint32_t textureRootParameterIndex, uint32_t instancingNum)
{
	for (size_t index = 0; index < modelResource_.modelData.size(); index++) {
		//頂点バッファービューを設定
		MainRender::GetInstance()->GetCommandList()->IASetVertexBuffers(0, 1, &modelResource_.vertexBufferView.at(index));
		//インデックスバッファビューを設定
		MainRender::GetInstance()->GetCommandList()->IASetIndexBuffer(&modelResource_.indexBufferView.at(index));
		//マテリアルCBufferの場所を設定
		MainRender::GetInstance()->GetCommandList()->SetGraphicsRootConstantBufferView(materialRootParameterIndex, modelResource_.materialResource.at(index)->GetGPUVirtualAddress());
		//モデルにテクスチャがない場合、スキップ
		if (modelResource_.modelData.at(index).material.textureFilePath.size() != 0) {
			//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]でテクスチャの設定をしているため。
			MainRender::GetInstance()->GetCommandList()->SetGraphicsRootDescriptorTable(textureRootParameterIndex, TextureManager::GetInstance()->GetSrvHandleGPU(modelResource_.modelData.at(index).material.textureHandle));
		}
		//描画
		MainRender::GetInstance()->GetCommandList()->DrawIndexedInstanced(UINT(modelResource_.modelData.at(index).indices.size()), instancingNum, 0, 0, 0);
	}
}

void Model::DrawLine(const WorldTransform& worldTransform, const BaseCamera& camera)
{
#ifdef _DEBUG
	if (isSkeleton_) {
		//全てのjoint(関節)を球で登録
		int lineIndex = 0;
		for (Joint& joint : skeleton_.joints) {
			Sphere sphere;
			Matrix4x4 matWorld = joint.skeletonSpaceMatrix * worldTransform.matWorld_;
			sphere.center = MyMath::Transform(joint.transform.translate, matWorld);
			sphere.radius = jointRadius_;

			// LineDrawerを使って球を描画
			MyMath::DrawSphere(sphere, Vector4(1, 1, 1, 1), lines_[lineIndex].get(), 5);

			lineIndex++;
		}
		//全てのjointのつなぎ(骨)を線で登録
		for (Joint& joint : skeleton_.joints) {
			Matrix4x4 matWorld = joint.skeletonSpaceMatrix * worldTransform.matWorld_;

			Vector3 start;
			Vector3 end;
			if (joint.parent) {
				start = MyMath::Transform(joint.transform.translate, matWorld);
				int32_t parentIndex = joint.parent.has_value();
				for (Joint& parent : skeleton_.joints) {
					if (parent.index == parentIndex) {
						end = MyMath::Transform(parent.transform.translate, matWorld);
						//LineDrawerを使って線を描画
						lines_[lineIndex]->CreateLine(start, end, Vector4(1, 1, 1, 1));
					}
				}
			}

		}

		//線を描画
		for (const auto& line : lines_) {
			line->Draw(camera);
		}
	}
#endif // _DEBUG
}

std::vector<Model::ModelData> Model::LoadModelFile()
{
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

		// メッシュ内の頂点データ等を解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); // 三角形のみサポート
			//頂点データの解析
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				aiVector3D& position = mesh->mVertices[vertexIndex];
				aiVector3D& normal = mesh->mNormals[vertexIndex];
				aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

				// 頂点データを格納
				VertexData vertex;
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
				vertex.texcoord = { texcoord.x, texcoord.y };

				// aiProcess_MakeLeftHandedの処理を手動で補正
				//vertex.position.x *= -1.0f;
				//vertex.normal.x *= -1.0f;

				//頂点データ
				model.vertices.push_back(vertex);
				//インデックス
				model.indices.push_back(vertexIndex);
			}
		}

		// 構築したモデルデータにルートノードを設定
		switch (mf_)
		{
		case OBJ:
			break;
		case GLTF:
			model.rootNode = ReadNode(scene->mRootNode);
			//スケルトン生成
			skeleton_ = CreateSkeleton(model.rootNode);
			isSkeleton_ = true;
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

Model::Animation Model::LoadAnimationFile()
{
	Animation animation;
	Assimp::Importer importer;
	std::string filePath = directoryPath_ + fileName_ + "/" + fileName_ + format_;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
	if (scene->mNumAnimations != 0) {
		isAnimation_ = true;
	}
	else {
		isAnimation_ = false;
		return animation;
	}
	aiAnimation* animationAssimp = scene->mAnimations[0];
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
		//translate
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			keyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { -keyAssimp.mValue.x,keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.translate.keyframes.push_back(keyframe);
		}
		//rotate
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			keyframeQuaternion keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x,-keyAssimp.mValue.y, -keyAssimp.mValue.z,keyAssimp.mValue.w };
			nodeAnimation.rotate.keyframes.push_back(keyframe);
		}
		//scale
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			keyframeVector3 keyframe;
			keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
			keyframe.value = { keyAssimp.mValue.x,keyAssimp.mValue.y, keyAssimp.mValue.z };
			nodeAnimation.scale.keyframes.push_back(keyframe);
		}

	}
	//解析完了
	return animation;
}

Model::Node Model::ReadNode(aiNode* node)
{
	// 新しいNodeオブジェクトを作成
	Node currentNode;

	// ノード名を設定
	currentNode.name = node->mName.C_Str();

	//変数宣言
	aiVector3D scale, translate;
	aiQuaternion rotate;

	node->mTransformation.Decompose(scale, rotate, translate);
	currentNode.transform.scale = { scale.x,scale.y,scale.z };
	currentNode.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w };
	currentNode.transform.translate = { -translate.x,translate.y,translate.z };
	currentNode.localMatrix = MyMath::MakeAffineMatrix(currentNode.transform.scale, currentNode.transform.rotate, currentNode.transform.translate);

	// 子ノードを再帰的に処理
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		currentNode.children.push_back(ReadNode(node->mChildren[i]));
	}

	return currentNode;
}

Model::ModelResource Model::MakeModelResource()
{
	//モデルリソース
	ModelResource modelResource_;

	modelResource_.modelData = LoadModelFile();
	animation_ = LoadAnimationFile();
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
		//リソースを作る
		modelResource_.vertexResource.at(index) = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * modelResource_.modelData.at(index).vertices.size());
		modelResource_.indexResource.at(index) = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) * modelResource_.modelData.at(index).indices.size());
		modelResource_.materialResource.at(index) = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Material));
		//頂点バッファービューを作成
		modelResource_.vertexBufferView.at(index).BufferLocation = modelResource_.vertexResource.at(index)->GetGPUVirtualAddress();
		modelResource_.vertexBufferView.at(index).SizeInBytes = UINT(sizeof(VertexData) * modelResource_.modelData.at(index).vertices.size());
		modelResource_.vertexBufferView.at(index).StrideInBytes = sizeof(VertexData);
		//インデックスバッファービューを作成
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

void Model::SettingTexture()
{
	for (size_t index = 0; index < modelResource_.modelData.size(); index++) {
		//.objの参照しているテクスチャファイル読み込み(TextureManagerはResources/をカットできるので)
		modelResource_.modelData.at(index).material.textureHandle = TextureManager::GetInstance()->LoadTexture("models/" + fileName_ + "/" + modelResource_.modelData.at(index).material.textureFilePath);
	}
}

Vector3 Model::CalculateValue(const std::vector<Keyframe<Vector3>>& keyframes, float time)
{
	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}
	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		//indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			//葉にないを補完する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return MyMath::Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}

	}
	//ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;
}

Quaternion Model::CalculateValue(const std::vector<Keyframe<Quaternion>>& keyframes, float time)
{
	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}
	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		//indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
		if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
			//葉にないを補完する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return MyMath::Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}

	}
	//ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
	return (*keyframes.rbegin()).value;

}

int32_t Model::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints)
{
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MyMath::MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size());
	joint.parent = parent;
	joints.push_back(joint);
	for (const Node& child : node.children) {
		//子Jointを作成し、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}
	//自身のIndexを返す
	return joint.index;
}

Model::Skeleton Model::CreateSkeleton(const Node& rootNode)
{
	Skeleton skeleton;
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	//名前とindexのマッピングを行いアクセスしやすくする
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;
}
