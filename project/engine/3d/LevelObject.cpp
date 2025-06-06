#include "LevelObject.h"
#include <assert.h>

void LevelObject::Initialize(const std::string& _filePath) {
	//jsonデータを読み込む
	json data = JsonUtil::GetJsonData(_filePath);
	//全オブジェクトデータの走査
	for (json& object : data["objects"]) {
		ScanObjectData(object);
	}


}

void LevelObject::Update() {
	//オブジェクトの更新
	for (const auto& object : objects_) {
		object->Update();
	}
}

void LevelObject::Draw() {
	//オブジェクトの描画
	for (const auto& object : objects_) {
		object->Draw(camera_);
	}
}

void LevelObject::DebugWithImGui() {
}

void LevelObject::ScanObjectData(json& object) {
	//"type"データがない場合不正データのため警告
	assert(object.contains("type"));

	//種別を取得
	std::string type = object["type"].get<std::string>();

	//メッシュオブジェクトの処理
	if (type.compare("MESH") == 0) {
		std::unique_ptr<Object3d> addObject;
		addObject = std::make_unique<Object3d>();
		addObject->Initialize(ModelTag{}, object["file_name"]);

		//トランスフォームパラメータの読み込み
		json& transform = object["transform"];
		//平行移動
		addObject->worldTransform.translate.x = (float)transform["translation"][0];
		addObject->worldTransform.translate.y = (float)transform["translation"][2];
		addObject->worldTransform.translate.z = (float)transform["translation"][1];
		//回転角
		addObject->worldTransform.rotate.x = -(float)transform["rotation"][0];
		addObject->worldTransform.rotate.y = -(float)transform["rotation"][2];
		addObject->worldTransform.rotate.z = -(float)transform["rotation"][1];
		//スケーリング
		addObject->worldTransform.scale.x = (float)transform["scaling"][0];
		addObject->worldTransform.scale.y = (float)transform["scaling"][2];
		addObject->worldTransform.scale.z = (float)transform["scaling"][1];

		//<!>コライダーは今はスキップ(システム考案中)

		//オブジェクトリストに登録
		objects_.push_back(std::move(addObject));
	}

	//子オブジェクトがある場合はその処理も行う(ペアレント処理は割愛→必要に応じて付け加える)
	if (object.contains("children")) {
		for (json& childObject : object["children"]) {
			ScanObjectData(childObject);
		}
	}
}
