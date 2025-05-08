#include "ParticleCreatorScene.h"
#include "SceneManager.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "ParticleManager.h"
#include "JsonUtil.h"
#include <filesystem>
#include <iostream>

void ParticleCreatorScene::Initialize() {
	//シーン共通の初期化
	BaseScene::Initialize();

	//インプット
	input_ = Input::GetInstance();

	//カメラの生成と初期化
	camera_ = std::make_unique<DevelopCamera>();
	camera_->Initialize();
	camera_->SetTranslate({ 0.0f,50.0f,0.0f });
	camera_->SetRotate({ 0.2f,0.0f,0.0f });
	//パーティクルマネージャーにカメラをセット
	ParticleManager::GetInstance()->SetCamera(camera_.get());

	//天球と地面の生成と初期化
	wtSkydome_.Initialize();
	skydome_ = std::make_unique<Object3d>();
	skydome_->InitializeModel("skydome");
	wtGround_.Initialize();
	textureHandleGround_ = TextureManager::GetInstance()->LoadTexture("grid.png");
	ground_ = std::make_unique<Object3d>();
	ground_->InitializeModel("ground");

	//エミッター可視化ラインの生成初期化
	lineEmitter_ = std::make_unique<LineDrawer>();
	lineEmitter_->Initialize();

}

void ParticleCreatorScene::Finalize() {
}

void ParticleCreatorScene::Update() {
	//シーン共通の更新
	BaseScene::Update();
	//カメラの更新
	camera_->Update();
	//モデルの更新
	wtSkydome_.UpdateMatrix();
	wtGround_.UpdateMatrix();

	//リセットコマンド
	if (!checkContinue_ && !checkSameName_ && !checkEditName_) {
		if (input_->TriggerKey(DIK_ESCAPE)) {
			isReset_ = true;
		}
	}

	//ImGui操作
#ifdef _DEBUG
	//最初の操作
	StartWithImGui();
	//新規作成の操作
	GenerateWithImGui();
	//編集の操作
	EditWithImGui();

	//リセット処理
	ResetWithImGui();
	//セーブ処理
	SaveWithImGui();

#endif // _DEBUG

}

void ParticleCreatorScene::Draw() {
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	//天球の描画
	skydome_->Draw(wtSkydome_, *camera_.get(), nullptr);
	//地面の描画
	ground_->Draw(wtGround_, *camera_.get(), nullptr, textureHandleGround_);

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
	///------------------------------///

	//線描画共通描画設定
	LineDrawerCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓線描画開始↓↓↓↓
	///------------------------------///

	//線描画
	lineEmitter_->Draw(*camera_.get());

	///------------------------------///
	///↑↑↑↑線描画終了↑↑↑↑
	///------------------------------///

}

void ParticleCreatorScene::TextDraw() {
}

void ParticleCreatorScene::StartWithImGui() {
#ifdef _DEBUG
	//最初の操作
	if (!isGenerateMode_ && !isEditMode_ && !checkEditName_) {
		ImGui::SetNextWindowPos(ImVec2(450, 300), ImGuiCond_FirstUseEver);
		ImGui::Begin("メニュー");
		ImGui::Text("パーティクルエディターへようこそ！\n以下の項目から希望のオプションを選択して下さい。\n");
		if (ImGui::Button("新しくパーティクルを作成する")) {
			isGenerateMode_ = true;
			//パーティクルの生成
			particle_ = std::make_unique<Particle>();
			//基本パーティクルをロード
			particle_->Initialize("Sample", "Basic");
			//エミッターの位置を調整
			particle_->emitter_.transform.translate = { 0.0f,3.0f,0.0f };
			//jsonデータをロード
			editParam_ = particle_->GetParam();
			//カメラの位置をセット
			camera_->SetTranslate({ 0.0f,4.0f,-20.0f });
			camera_->SetRotate({ 0.03f,0.0f,0.0f });
		}
		if (ImGui::Button("既存のパーティクルを編集する")) {
			checkEditName_ = true;
			//パーティクルの生成
			particle_ = std::make_unique<Particle>();
		}
		ImGui::End();
	}

#endif // _DEBUG
}

void ParticleCreatorScene::GenerateWithImGui() {
#ifdef _DEBUG
	if (isGenerateMode_ && !isEditMode_) {
		Editor();
	}
#endif // _DEBUG
}

void ParticleCreatorScene::EditWithImGui() {
#ifdef _DEBUG
	//パーティクル名の入力
	if (checkEditName_) {
		ImGui::OpenPopup("パーティクル名の入力");
	}
	ImGui::SetWindowSize(ImVec2(510, 120));
	if (ImGui::BeginPopupModal("パーティクル名の入力", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::Text("{particles}フォルダ内のJsonファイル名を入力してください(.jsonは省略)\n ");
		//particlesから参照
		if (ImGui::Button("{particles}フォルダから参照する")) {
			showFileDialog_ = true;
			particleFiles_.clear();
			for (const auto& entry : std::filesystem::directory_iterator("Resources/particles")) {
				if (entry.is_regular_file() && entry.path().extension() == ".json") {
					particleFiles_.push_back(entry.path().filename().string());
				}
			}
		}
		char buffer[256];
		strncpy_s(buffer, sizeof(buffer), jsonFileName_.c_str(), _TRUNCATE);
		buffer[sizeof(buffer) - 1] = '\0';
		//入力欄
		if (ImGui::InputText("入力欄", buffer, sizeof(buffer))) {
			jsonFileName_ = buffer;
		}
		ImGui::Text("入力されている名前\n%s\n ", jsonFileName_.c_str());

		//検索済みのパーティクル一覧を表示
		if (showFileDialog_) {
			ImGui::SetNextWindowFocus();
			ImGui::SetNextWindowPos(ImVec2(860, 280));
			ImGui::Begin("パーティクル一覧");
			for (const auto& file : particleFiles_) {
				if (ImGui::Selectable(file.c_str())) {
					//.jsonをカット
					std::string cutJson = std::filesystem::path(file).stem().string();
					jsonFileName_ = cutJson;  // 選択したファイル名を保存
					showFileDialog_ = false;  // ウィンドウを閉じる
				}
			}
			ImGui::End();
		}
		//確定ボタン
		if (jsonFileName_.size() != 0) {
			//particleフォルダ内にあるか照合
			if (JsonUtil::CheckJson(jsonFileName_, "Resources/particles/")) {
				//ちゃんと見つかった
				checkSameName_ = true;
			}
			else {
				//見つからない
				checkSameName_ = false;
			}
			//名前があるかで決める処理
			if (checkSameName_) {
				//ある場合
				if (ImGui::Button("このファイルを編集する")) {
					checkSameName_ = false;
					checkEditName_ = false;
					isEditMode_ = true;
					//基本パーティクルをロード
					particle_->Initialize("Sample", jsonFileName_);
					//エミッターの位置を調整
					particle_->emitter_.transform.translate = { 0.0f,3.0f,0.0f };
					//jsonデータをロード
					editParam_ = particle_->GetParam();
					//カメラの位置をセット
					camera_->SetTranslate({ 0.0f,4.0f,-20.0f });
					camera_->SetRotate({ 0.03f,0.0f,0.0f });

					ImGui::CloseCurrentPopup();
				}
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.8f, 0.3f, 1.0f));
				ImGui::Text("指定されたファイルが見つかりました");
				ImGui::PopStyleColor();
			}
			else {
				//ない場合
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
				ImGui::Text("指定されたファイルが存在しません");
				ImGui::PopStyleColor();
			}
		}
		//戻る
		if (ImGui::Button("Back")) {
			showFileDialog_ = false;
			checkSameName_ = false;
			checkEditName_ = false;
			jsonFileName_ = std::string();
			particle_.release();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	//編集モード本番
	if (isEditMode_ && !isGenerateMode_) {
		Editor();
	}
#endif // _DEBUG
}

void ParticleCreatorScene::ResetWithImGui() {
#ifdef _DEBUG
	if (isReset_) {
		ImGui::OpenPopup("確認");
	}
	ImGui::SetNextWindowPos(ImVec2(510, 30));
	if (ImGui::BeginPopupModal("確認", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::Text("リセットしますか？\n[注意] 編集中のパラメーターは反映されません\n ");
		if (ImGui::Button("はい", ImVec2(120, 0))) {
			sceneManager_->SetNextScene("PARTICLECREATOR");
			isReset_ = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("いいえ", ImVec2(120, 0))) {
			isReset_ = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
#endif // _DEBUG

}

void ParticleCreatorScene::SaveWithImGui() {
#ifdef _DEBUG
	if (isSave_) {
		ImGui::SetNextWindowPos(ImVec2(470, 280));
		ImGui::Begin("パーティクルのセーブ");

		ImGui::Text("作成したパーティクルの名前を入力してください(.jsonは省略)");
		char buffer[256];
		strncpy_s(buffer, sizeof(buffer), jsonFileName_.c_str(), _TRUNCATE);
		buffer[sizeof(buffer) - 1] = '\0';
		if (ImGui::InputText("入力欄", buffer, sizeof(buffer))) {
			jsonFileName_ = buffer;
		}
		ImGui::Text("入力されている名前\n%s\n ", jsonFileName_.c_str());
		if (jsonFileName_.size() != 0) {
			if (ImGui::Button("名前を確定する")) {
				//JsonUtilを使ってパーティクルを保存
				if (JsonUtil::CreateJson(jsonFileName_, "Resources/particles/", editParam_)) {
					checkContinue_ = true;
				}
				//すでに同名ファイルがある
				else {
					checkSameName_ = true;
				}
			}
		}
		//続行するかチェック
		if (checkContinue_) {
			ImGui::OpenPopup("パーティクルの作成に成功しました");
		}
		ImGui::SetNextWindowPos(ImVec2(510, 30));
		if (ImGui::BeginPopupModal("パーティクルの作成に成功しました", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::Text("作業を終了しますか？\n ");
			if (ImGui::Button("はい", ImVec2(120, 0))) {
				sceneManager_->SetNextScene("PARTICLECREATOR");
				checkContinue_ = false;
				isSave_ = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("いいえ", ImVec2(120, 0))) {
				checkContinue_ = false;
				isSave_ = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		//同名ファイルがあった時の処理
		if (checkSameName_) {
			ImGui::OpenPopup("同名のファイルが見つかりました");
		}
		ImGui::SetNextWindowPos(ImVec2(510, 30));
		if (ImGui::BeginPopupModal("同名のファイルが見つかりました", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
			ImGui::Text("上書きしますか？\n ");
			if (ImGui::Button("はい", ImVec2(120, 0))) {
				//JsonUtilで既存のファイルを編集
				std::string fullPath = "Resources/particles/" + jsonFileName_;
				JsonUtil::EditJson(fullPath, editParam_);
				checkSameName_ = false;
				checkContinue_ = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("いいえ", ImVec2(120, 0))) {
				checkSameName_ = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		//表示を消す
		if (ImGui::Button("Back")) {
			isSave_ = false;
		}
		ImGui::End();
	}
#endif // _DEBUG
}

void ParticleCreatorScene::Editor() {
#ifdef _DEBUG
	//パラメーター
	ImGui::SetNextWindowPos(ImVec2(10, 80), ImGuiCond_FirstUseEver);
	ImGui::Begin("パーティクルのパラメーター");
	//テクスチャを写す
	std::string selectedTexture = particle_->GetParam()["Texture"];
	if (ImGui::CollapsingHeader("テクスチャの設定")) {
		ImGui::Text("選択中のテクスチャ : %s", selectedTexture.c_str());
		//フォルダ内のテクスチャを検索
		if (ImGui::Button("{textures}フォルダ内のテクスチャを検索")) {
			showFileDialog_ = true;
			textureFiles_.clear();
			for (const auto& entry : std::filesystem::directory_iterator("Resources/textures")) {
				if (entry.is_regular_file() && entry.path().extension() == ".png") {
					textureFiles_.push_back(entry.path().filename().string());
				}
			}
		}
		//検索済みのテクスチャ一覧を表示
		if (showFileDialog_) {
			ImGui::SetNextWindowPos(ImVec2(360, 80));
			ImGui::Begin("テクスチャ一覧");
			for (const auto& file : textureFiles_) {
				if (ImGui::Selectable(file.c_str())) {
					selectedTexture = file;  // 選択したファイル名を保存
					showFileDialog_ = false;  // ウィンドウを閉じる
				}
			}
			ImGui::End();
		}

	}

	//色を写す
	Vector4 startColorMax = { editParam_["StartColor"]["Max"]["x"],editParam_["StartColor"]["Max"]["y"],editParam_["StartColor"]["Max"]["z"],editParam_["StartColor"]["Max"]["w"] };
	Vector4 startColorMin = { editParam_["StartColor"]["Min"]["x"],editParam_["StartColor"]["Min"]["y"],editParam_["StartColor"]["Min"]["z"],editParam_["StartColor"]["Min"]["w"] };
	Vector4 endColorMax = { editParam_["EndColor"]["Max"]["x"],editParam_["EndColor"]["Max"]["y"],editParam_["EndColor"]["Max"]["z"],editParam_["EndColor"]["Max"]["w"] };
	Vector4 endColorMin = { editParam_["EndColor"]["Min"]["x"],editParam_["EndColor"]["Min"]["y"],editParam_["EndColor"]["Min"]["z"],editParam_["EndColor"]["Min"]["w"] };
	if (ImGui::CollapsingHeader("色の設定")) {
		ImGui::ColorEdit4("StartColorMax", &startColorMax.x);
		ImGui::ColorEdit4("StartColorMin", &startColorMin.x);
		ImGui::ColorEdit4("EndColorMax", &endColorMax.x);
		ImGui::ColorEdit4("EndColorMin", &endColorMin.x);
	}
	//トランスフォームを写す
	Vector3 rotateMax = { editParam_["GrainTransform"]["Rotate"]["Max"]["x"],editParam_["GrainTransform"]["Rotate"]["Max"]["y"] ,editParam_["GrainTransform"]["Rotate"]["Max"]["z"] };
	Vector3 rotateMin = { editParam_["GrainTransform"]["Rotate"]["Min"]["x"],editParam_["GrainTransform"]["Rotate"]["Min"]["y"] ,editParam_["GrainTransform"]["Rotate"]["Min"]["z"] };
	Vector3 scaleMax = { editParam_["GrainTransform"]["Scale"]["Max"]["x"],editParam_["GrainTransform"]["Scale"]["Max"]["y"],editParam_["GrainTransform"]["Scale"]["Max"]["z"] };
	Vector3 scaleMin = { editParam_["GrainTransform"]["Scale"]["Min"]["x"],editParam_["GrainTransform"]["Scale"]["Min"]["y"],editParam_["GrainTransform"]["Scale"]["Min"]["z"] };
	if (ImGui::CollapsingHeader("トランスフォームの設定")) {
		if (ImGui::TreeNode("Rotate")) {
			ImGui::DragFloat3("RotateMax", &rotateMax.x, 0.1f);
			ImGui::DragFloat3("RotateMin", &rotateMin.x, 0.1f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Scale")) {
			ImGui::DragFloat3("ScaleMax", &scaleMax.x, 0.1f, 0.0f, 10.0f);
			ImGui::DragFloat3("ScaleMin", &scaleMin.x, 0.1f, 0.0f, 10.0f);
			ImGui::TreePop();
		}
	}
	//サイズを写す
	float startSizeMax = editParam_["StartSize"]["Max"];
	float startSizeMin = editParam_["StartSize"]["Min"];
	float endSizeMax = editParam_["EndSize"]["Max"];
	float endSizeMin = editParam_["EndSize"]["Min"];
	if (ImGui::CollapsingHeader("サイズの設定")) {
		ImGui::DragFloat("StartSizeMax", &startSizeMax, 0.1f, startSizeMin, 10.0f);
		ImGui::DragFloat("StartSizeMin", &startSizeMin, 0.1f, 0.0f, startSizeMax);
		ImGui::DragFloat("EndSizeMax", &endSizeMax, 0.1f, endSizeMin, 10.0f);
		ImGui::DragFloat("EndSizeMin", &endSizeMin, 0.1f, 0.0f, endSizeMax);
	}
	//速度を写す
	Vector3 velocityMax = { editParam_["Velocity"]["Max"]["x"],editParam_["Velocity"]["Max"]["y"],editParam_["Velocity"]["Max"]["z"] };
	Vector3 velocityMin = { editParam_["Velocity"]["Min"]["x"],editParam_["Velocity"]["Min"]["y"],editParam_["Velocity"]["Min"]["z"] };
	if (ImGui::CollapsingHeader("速度の設定")) {
		ImGui::DragFloat3("VelocityMax", &velocityMax.x, 0.1f);
		ImGui::DragFloat3("VelocityMin", &velocityMin.x, 0.1f);
	}
	//寿命を写す
	float lifeTimeMax = editParam_["LifeTime"]["Max"];
	float lifeTimeMin = editParam_["LifeTime"]["Min"];
	if (ImGui::CollapsingHeader("寿命の設定")) {
		ImGui::DragFloat("LifeTimeMax", &lifeTimeMax, 0.1f, lifeTimeMin, 30.0f);
		ImGui::DragFloat("LifeTimeMin", &lifeTimeMin, 0.1f, 0.0f, lifeTimeMax);
	}
	//粒の最大数を写す
	int maxGrains = editParam_["MaxGrains"];
	if (ImGui::CollapsingHeader("粒の最大数")) {
		//推奨値の計算
		int RecommendValue;
		switch (particle_->emitter_.generateMethod) {
		case Particle::GenerateMethod::kRandom:
			RecommendValue = int(lifeTimeMax * editParam_["EmitRate"]);
			break;
		case Particle::GenerateMethod::kClump:
			RecommendValue = int(lifeTimeMax * editParam_["EmitRate"] * particle_->emitter_.clumpNum);
			break;
		default:
			break;
		}
		ImGui::Text("推奨値 : %d (最低限の数で回せる値)", RecommendValue);
		if (ImGui::Button("推奨値を適用")) {
			maxGrains = RecommendValue;
		}
		ImGui::SliderInt("MaxGrains", &maxGrains, 1, 1000);
	}
	//1秒あたりの粒の生成量を写す
	int emitRate = editParam_["EmitRate"];
	if (ImGui::CollapsingHeader("1秒あたりの粒の生成量")) {
		ImGui::SliderInt("EmitRate", &emitRate, 1, 100);
	}
	//ブレンドモードを写す
	int blendMode = editParam_["BlendMode"];
	const char* blendModeList[] = { "None","Normal","Add","Subtract","Multiply","Screen","Exclusion" };
	if (ImGui::CollapsingHeader("ブレンドモード")) {
		ImGui::Combo("BlendMode", &blendMode, blendModeList, (int)BlendMode::kMaxBlendModeNum);
	}
	//プリミティブを写す
	int primitive = editParam_["Primitive"];
	bool isShapeChange = false;
	const char* primitiveList[] = { "Shere","Cube","SkyBox","Plane","Ring","Tube" };
	if (ImGui::CollapsingHeader("形状")) {
		if (ImGui::Combo("Primitive", &primitive, primitiveList, (int)Shape::ShapeKind::kMaxShapeKindNum)) {
			//形状の変更通知
			isShapeChange = true;
		}
	}

	//editParamに変更を反映
	editParam_["Texture"] = selectedTexture;
	editParam_["StartColor"]["Max"]["x"] = startColorMax.x;
	editParam_["StartColor"]["Max"]["y"] = startColorMax.y;
	editParam_["StartColor"]["Max"]["z"] = startColorMax.z;
	editParam_["StartColor"]["Max"]["w"] = startColorMax.w;
	editParam_["StartColor"]["Min"]["x"] = startColorMin.x;
	editParam_["StartColor"]["Min"]["y"] = startColorMin.y;
	editParam_["StartColor"]["Min"]["z"] = startColorMin.z;
	editParam_["StartColor"]["Min"]["w"] = startColorMin.w;
	editParam_["EndColor"]["Max"]["x"] = endColorMax.x;
	editParam_["EndColor"]["Max"]["y"] = endColorMax.y;
	editParam_["EndColor"]["Max"]["z"] = endColorMax.z;
	editParam_["EndColor"]["Max"]["w"] = endColorMax.w;
	editParam_["EndColor"]["Min"]["x"] = endColorMin.x;
	editParam_["EndColor"]["Min"]["y"] = endColorMin.y;
	editParam_["EndColor"]["Min"]["z"] = endColorMin.z;
	editParam_["EndColor"]["Min"]["w"] = endColorMin.w;
	editParam_["GrainTransform"]["Rotate"]["Max"]["x"] = rotateMax.x;
	editParam_["GrainTransform"]["Rotate"]["Max"]["y"] = rotateMax.y;
	editParam_["GrainTransform"]["Rotate"]["Max"]["z"] = rotateMax.z;
	editParam_["GrainTransform"]["Rotate"]["Min"]["x"] = rotateMin.x;
	editParam_["GrainTransform"]["Rotate"]["Min"]["y"] = rotateMin.y;
	editParam_["GrainTransform"]["Rotate"]["Min"]["z"] = rotateMin.z;
	editParam_["GrainTransform"]["Scale"]["Max"]["x"] = scaleMax.x;
	editParam_["GrainTransform"]["Scale"]["Max"]["y"] = scaleMax.y;
	editParam_["GrainTransform"]["Scale"]["Max"]["z"] = scaleMax.z;
	editParam_["GrainTransform"]["Scale"]["Min"]["x"] = scaleMin.x;
	editParam_["GrainTransform"]["Scale"]["Min"]["y"] = scaleMin.y;
	editParam_["GrainTransform"]["Scale"]["Min"]["z"] = scaleMin.z;
	editParam_["StartSize"]["Max"] = startSizeMax;
	editParam_["StartSize"]["Min"] = startSizeMin;
	editParam_["EndSize"]["Max"] = endSizeMax;
	editParam_["EndSize"]["Min"] = endSizeMin;
	editParam_["Velocity"]["Max"]["x"] = velocityMax.x;
	editParam_["Velocity"]["Max"]["y"] = velocityMax.y;
	editParam_["Velocity"]["Max"]["z"] = velocityMax.z;
	editParam_["Velocity"]["Min"]["x"] = velocityMin.x;
	editParam_["Velocity"]["Min"]["y"] = velocityMin.y;
	editParam_["Velocity"]["Min"]["z"] = velocityMin.z;
	editParam_["LifeTime"]["Max"] = lifeTimeMax;
	editParam_["LifeTime"]["Min"] = lifeTimeMin;
	editParam_["MaxGrains"] = maxGrains;
	editParam_["EmitRate"] = emitRate;
	editParam_["BlendMode"] = blendMode;
	editParam_["Primitive"] = primitive;
	//パーティクルに反映
	particle_->SetParam(editParam_);
	//形状の変更通知を受け取ったら
	if (isShapeChange) {
		//パーティクルの形状を変更
		particle_->ShapeChange();
	}

	//セーブボタン
	if (ImGui::Button("セーブ")) {
		isSave_ = true;
	}
	ImGui::End();

	//エミッター
	ImGui::SetNextWindowPos(ImVec2(1000, 80), ImGuiCond_FirstUseEver);
	ImGui::Begin("エミッター");
	//エミッター可視化
	ImGui::Checkbox("エミッターの枠を表示する", &displayLineEmitter_);
	if (displayLineEmitter_) {
		Vector3 prbf;		//右下前
		Vector3 plbf;		//左下前
		Vector3 prtf;		//右上前
		Vector3 pltf;		//左上前
		Vector3 prbb;		//右下奥
		Vector3 plbb;		//左下奥
		Vector3 prtb;		//右上奥
		Vector3 pltb;		//左上奥
		prbf = {
			particle_->emitter_.transform.translate.x + particle_->emitter_.transform.scale.x,
			particle_->emitter_.transform.translate.y - particle_->emitter_.transform.scale.y,
			particle_->emitter_.transform.translate.z - particle_->emitter_.transform.scale.z
		};
		plbf = {
			particle_->emitter_.transform.translate.x - particle_->emitter_.transform.scale.x,
			particle_->emitter_.transform.translate.y - particle_->emitter_.transform.scale.y,
			particle_->emitter_.transform.translate.z - particle_->emitter_.transform.scale.z
		};
		prtf = {
			particle_->emitter_.transform.translate.x + particle_->emitter_.transform.scale.x,
			particle_->emitter_.transform.translate.y + particle_->emitter_.transform.scale.y,
			particle_->emitter_.transform.translate.z - particle_->emitter_.transform.scale.z
		};
		pltf = {
			particle_->emitter_.transform.translate.x - particle_->emitter_.transform.scale.x,
			particle_->emitter_.transform.translate.y + particle_->emitter_.transform.scale.y,
			particle_->emitter_.transform.translate.z - particle_->emitter_.transform.scale.z
		};
		prbb = {
			particle_->emitter_.transform.translate.x + particle_->emitter_.transform.scale.x,
			particle_->emitter_.transform.translate.y - particle_->emitter_.transform.scale.y,
			particle_->emitter_.transform.translate.z + particle_->emitter_.transform.scale.z
		};
		plbb = {
			particle_->emitter_.transform.translate.x - particle_->emitter_.transform.scale.x,
			particle_->emitter_.transform.translate.y - particle_->emitter_.transform.scale.y,
			particle_->emitter_.transform.translate.z + particle_->emitter_.transform.scale.z
		};
		prtb = {
			particle_->emitter_.transform.translate.x + particle_->emitter_.transform.scale.x,
			particle_->emitter_.transform.translate.y + particle_->emitter_.transform.scale.y,
			particle_->emitter_.transform.translate.z + particle_->emitter_.transform.scale.z
		};
		pltb = {
			particle_->emitter_.transform.translate.x - particle_->emitter_.transform.scale.x,
			particle_->emitter_.transform.translate.y + particle_->emitter_.transform.scale.y,
			particle_->emitter_.transform.translate.z + particle_->emitter_.transform.scale.z
		};
		Vector4 color = { 1,0,0,1 };

		lineEmitter_->CreateLine(prbf, plbf, color);
		lineEmitter_->CreateLine(plbf, pltf, color);
		lineEmitter_->CreateLine(pltf, prtf, color);
		lineEmitter_->CreateLine(prtf, prbf, color);

		lineEmitter_->CreateLine(prbb, plbb, color);
		lineEmitter_->CreateLine(plbb, pltb, color);
		lineEmitter_->CreateLine(pltb, prtb, color);
		lineEmitter_->CreateLine(prtb, prbb, color);

		lineEmitter_->CreateLine(prbf, prbb, color);
		lineEmitter_->CreateLine(plbf, plbb, color);
		lineEmitter_->CreateLine(prtf, prtb, color);
		lineEmitter_->CreateLine(pltf, pltb, color);
	}
	//エミッターのトランスフォーム
	if (ImGui::CollapsingHeader("エミッターのトランスフォーム")) {
		ImGui::DragFloat3("平行移動", &particle_->emitter_.transform.translate.x, 0.1f);
		ImGui::DragFloat3("拡縮", &particle_->emitter_.transform.scale.x, 0.1f);
	}
	//生成アルゴリズム
	if (ImGui::CollapsingHeader("生成アルゴリズム")) {
		ImGui::Checkbox("生成するか(isPlay)", &particle_->emitter_.isPlay);
		if (ImGui::TreeNode("生成方法")) {
			//現在の生成方法を表示
			const char* methods[] = { "Random","Clump" };
			const char* currentMethod = "";
			switch (particle_->emitter_.generateMethod) {
			case Particle::GenerateMethod::kRandom:
				currentMethod = methods[0];
				break;
			case Particle::GenerateMethod::kClump:
				currentMethod = methods[1];
				break;
			default:
				break;
			}
			ImGui::Text("現在の生成方法 : %s", currentMethod);
			//生成方法の選択
			ImGui::Combo("生成方法(generateMethod)", (int*)&particle_->emitter_.generateMethod, methods, IM_ARRAYSIZE(methods));
			//生成方法ごとの設定
			switch (particle_->emitter_.generateMethod) {
			case Particle::GenerateMethod::kRandom:
				break;
			case Particle::GenerateMethod::kClump:
				ImGui::DragInt("一塊の粒の数(clumpNum)", &particle_->emitter_.clumpNum, 1, 1, 20);

				break;
			default:
				break;
			}

			ImGui::TreePop();
		}
	}
	//重力関係
	if (ImGui::CollapsingHeader("重力")) {
		ImGui::Checkbox("重力の処理をするか(isGravity)", &particle_->emitter_.isGravity);
		ImGui::DragFloat("重力値", &particle_->emitter_.gravity, 0.1f);
	}
	//床関係
	if (ImGui::CollapsingHeader("床")) {
		ImGui::Checkbox("床の処理をするか(isBound)", &particle_->emitter_.isBound);
		ImGui::DragFloat("床の反発値", &particle_->emitter_.repulsion, 0.1f);
		ImGui::DragFloat("床の高さ", &particle_->emitter_.floorHeight, 0.1f);
	}
	//ビルボードを適用するか
	if (ImGui::CollapsingHeader("ビルボード")) {
		ImGui::Checkbox("ビルボードの処理をするか(isBillboard)", &particle_->emitter_.isBillboard);
	}
	ImGui::End();
#endif // _DEBUG
}
