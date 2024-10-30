#include "Particle.h"
#include "ParticleCommon.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "externals/imgui/imgui.h"
#include <fstream>
#include <sstream>
#include <random>

void Particle::Initialize(ParticleCommon* modelCommon)
{
	//引数で受け取ってメンバ変数に記録する
	particleCommon_ = modelCommon;

	//一旦planeでリソースを作る
	MakeModelResource("Resources", "plane.obj");
	SettingTexture();
	SettingSRV();

	//平行光源用リソースを作る
	directionalLightResource = particleCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::DirectionalLight));
	//リソースにデータをセット
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	//平行光源用データ
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 0.0f, -1.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	//エミッター生成
	emitter.transform.scale = { 1.0f,1.0f,1.0f };
	emitter.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter.transform.translate = { 0.0f,0.0f,0.0f };
	emitter.count = 3;//1度に3個生成する
	emitter.frequency = 0.5f;//0.5秒ごとに発生
	emitter.frequencyTime = 0.0f;//currentTime

	//フィールド生成
	accelerationField.acceleration = { 15.0f,0.0f,0.0f };
	accelerationField.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField.area.max = { 1.0f,1.0f,1.0f };

}

void Particle::Update()
{
	//インスタンスの番号
	uint32_t instanceNum = 0;
	//エミッターの更新
	emitter.frequencyTime += kDeltaTime;
	if (emitter.frequency <= emitter.frequencyTime) {
		particles.splice(particles.end(), Emit(emitter));
		emitter.frequencyTime -= emitter.frequency;
	}

	for (std::list<Struct::Particle>::iterator particleIterator = particles.begin(); particleIterator != particles.end();) {
		//時間更新
		++(*particleIterator).currentTime;

		//生存チェック
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			//寿命を迎えたら削除
			particleIterator = particles.erase(particleIterator);
			continue;
		}

		//フィールドの処理
		if (isField) {
			if (IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
				(*particleIterator).velocity = Add((*particleIterator).velocity, Multiply(kDeltaTime, accelerationField.acceleration));
			}
		}

		//速度加算処理
		(*particleIterator).transform.translate = Add((*particleIterator).transform.translate, Multiply(kDeltaTime, (*particleIterator).velocity));
		//α値設定
		float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);

		//レンダリングパイプライン
		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
		Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, cameraMatrix);
		billboardMatrix.m[3][0] = 0.0f;
		billboardMatrix.m[3][1] = 0.0f;
		billboardMatrix.m[3][2] = 0.0f;
		Matrix4x4 worldMatrix = Multiply(Multiply(MakeScaleMatrix((*particleIterator).transform.scale), billboardMatrix), MakeTranslateMatrix((*particleIterator).transform.translate));
		if (!isBillboard) {
			worldMatrix = MakeAffineMatrix((*particleIterator).transform.scale, (*particleIterator).transform.rotate, (*particleIterator).transform.translate);
		}
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(WinApp::kClientWidth) / float(WinApp::kClientHeight), 0.1f, 100.0f);
		Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		particleResource_.instancingData[instanceNum].WVP = worldViewProjectionMatrix;
		particleResource_.instancingData[instanceNum].World = worldMatrix;
		particleResource_.instancingData[instanceNum].color = (*particleIterator).color;
		particleResource_.instancingData[instanceNum].color.w = alpha;

		//インスタンスの数インクリメント
		++instanceNum;
		//次のイテレータに進める
		++particleIterator;
	}

#ifdef _DEBUG
	ImGui::Begin("particle");
	ImGui::Checkbox("billboard", &isBillboard);
	ImGui::Checkbox("field", &isField);
	if (ImGui::Button("Add Particle")) {
		//パーティクル生成
		particles.splice(particles.end(), Emit(emitter));
	}
	ImGui::DragFloat3("EmitterTranslate", &emitter.transform.translate.x, 0.01f, -100.0f, 100.0f);
	ImGui::End();
#endif // _DEBUG


}

void Particle::Draw()
{
	for (size_t index = 0; index < particleResource_.modelData.size(); index++) {
		//頂点バッファービューを設定
		particleCommon_->GetDirectXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &particleResource_.vertexBufferView.at(index));

		//マテリアルCBufferの場所を設定
		particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, particleResource_.materialResource.at(index)->GetGPUVirtualAddress());
		//座標変換行列CBufferの場所を設定
		particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(1, SrvHandleGPU);
		//モデルにテクスチャがない場合、スキップ
		if (particleResource_.modelData.at(index).material.textureFilePath.size() != 0) {
			//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]でテクスチャの設定をしているため。
			particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(particleResource_.modelData.at(index).material.textureIndex));
		}

		//平行光源の設定
		particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		//描画
		particleCommon_->GetDirectXCommon()->GetCommandList()->DrawInstanced(UINT(particleResource_.modelData.at(index).vertices.size()), particles.size(), 0, 0);
	}
}

Particle::Struct::MaterialData Particle::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName, const std::string& materialName)
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

std::vector<Particle::Struct::ModelData> Particle::LoadObjFile(const std::string& directoryPath, const std::string& fileName)
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

void Particle::MakeModelResource(const std::string& resourceFileName, const std::string& objFileName)
{
	particleResource_.modelData = LoadObjFile(resourceFileName, objFileName);
	const size_t kModelNum = particleResource_.modelData.size();
	//std::vector型の要素数を確定
	particleResource_.vertexResource.resize(kModelNum);
	particleResource_.vertexBufferView.resize(kModelNum);
	particleResource_.vertexData.resize(kModelNum);
	particleResource_.materialResource.resize(kModelNum);
	particleResource_.materialData.resize(kModelNum);
	particleResource_.textureResorce.resize(kModelNum);
	particleResource_.textureSrvHandleCPU.resize(kModelNum);
	particleResource_.textureSrvHandleGPU.resize(kModelNum);
	particleResource_.uvTransform.resize(kModelNum);
	for (size_t index = 0; index < kModelNum; index++) {
		//頂点用リソースを作る
		particleResource_.vertexResource.at(index) = particleCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::VertexData) * particleResource_.modelData.at(index).vertices.size());
		//マテリアル用のリソースを作る。
		particleResource_.materialResource.at(index) = particleCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::Material));
		//インスタンス用のリソースを作る
		particleResource_.instancingResource = particleCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Struct::ParticleForGPU) * kNumMaxInstance_);

		//頂点バッファービューを作成
		particleResource_.vertexBufferView.at(index).BufferLocation = particleResource_.vertexResource.at(index)->GetGPUVirtualAddress();
		particleResource_.vertexBufferView.at(index).SizeInBytes = UINT(sizeof(Struct::VertexData) * particleResource_.modelData.at(index).vertices.size());
		particleResource_.vertexBufferView.at(index).StrideInBytes = sizeof(Struct::VertexData);

		//リソースにデータを書き込む
		particleResource_.vertexResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&particleResource_.vertexData.at(index)));
		std::memcpy(particleResource_.vertexData.at(index), particleResource_.modelData.at(index).vertices.data(), sizeof(Struct::VertexData) * particleResource_.modelData.at(index).vertices.size());
		particleResource_.materialResource.at(index)->Map(0, nullptr, reinterpret_cast<void**>(&particleResource_.materialData.at(index)));
		particleResource_.instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&particleResource_.instancingData));

		//マテリアルデータ書き込み
		particleResource_.materialData.at(index)->color = particleResource_.modelData.at(index).material.colorData;
		particleResource_.materialData.at(index)->lightingKind = NoneLighting;
		//インスタンスデータ書き込み
		for (uint32_t index = 0; index < kNumMaxInstance_; ++index) {
			particleResource_.instancingData[index].WVP = MakeIdentity4x4();
			particleResource_.instancingData[index].World = MakeIdentity4x4();
			particleResource_.instancingData[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		//uvTransform
		particleResource_.materialData.at(index)->uvTransform = MakeIdentity4x4();
		//テクスチャを持っているか
		bool isTexture = true;
		if (particleResource_.modelData.at(index).material.textureFilePath.size() == 0) {
			//テクスチャファイルパスに書き込まれていない→テクスチャがない
			isTexture = false;
		}
		particleResource_.materialData.at(index)->isTexture = isTexture;
		//トランスフォーム
		particleResource_.transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
		};

		//UVトランスフォーム
		particleResource_.uvTransform.at(index) = {
			{1.0f,1.0f,1.0f},
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f}
		};
	}
}

void Particle::SettingTexture()
{
	for (size_t index = 0; index < particleResource_.modelData.size(); index++) {
		//.objの参照しているテクスチャファイル読み込み
		TextureManager::GetInstance()->LoadTexture(particleResource_.modelData.at(index).material.textureFilePath);
		//読み込んだテクスチャの番号を取得
		particleResource_.modelData.at(index).material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(particleResource_.modelData.at(index).material.textureFilePath);
	}
}

void Particle::SettingSRV()
{
	//srv設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = kNumMaxInstance_;
	srvDesc.Buffer.StructureByteStride = sizeof(Struct::ParticleForGPU);
	SrvHandleCPU = particleCommon_->GetDirectXCommon()->GetSRVCPUDescriptorHandle(4);
	SrvHandleGPU = particleCommon_->GetDirectXCommon()->GetSRVGPUDescriptorHandle(4);
	particleCommon_->GetDirectXCommon()->GetDevice()->CreateShaderResourceView(particleResource_.instancingResource.Get(), &srvDesc, SrvHandleCPU);
}

Particle::Struct::Particle Particle::MakeNewParticle(const Vector3& translate)
{
	Struct::Particle particle;
	//ランダムエンジンの生成
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());
	//トランスフォーム
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	Vector3 randomTranslate = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.transform.translate = Add(translate, randomTranslate);
	//速度
	particle.velocity = { distribution(randomEngine) ,distribution(randomEngine) ,distribution(randomEngine) };
	//色
	std::uniform_real_distribution<float> distcolor(0.0f, 1.0f);
	particle.color = { distcolor(randomEngine) ,distcolor(randomEngine) ,distcolor(randomEngine),1.0f };
	//寿命
	std::uniform_real_distribution<float> distTime(1.0f * 60.0f, 3.0f * 60.0f);
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;

	return particle;
}

std::list<Particle::Struct::Particle> Particle::Emit(const Struct::Emitter& emitter)
{
	std::list<Struct::Particle> particle;

	for (uint32_t count = 0; count < emitter.count; ++count) {
		particle.push_back(MakeNewParticle(emitter.transform.translate));
	}

	return particle;
}
