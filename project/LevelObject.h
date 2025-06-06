#pragma once
#include "BaseCamera.h"
#include "Object3d.h"
#include "JsonUtil.h"
#include <list>
#include <string>
#include <memory>

class LevelObject {
public:
	//初期化(.jsonは省略)
	void Initialize(const std::string& _filePath);
	//更新
	void Update();
	//描画
	void Draw();
	//デバッグ
	void DebugWithImGui();

public://setter
	void SetCamera(BaseCamera* _camera) {
		camera_ = _camera;
	}

private://クラス内限定処理
	//オブジェクトデータの走査
	void ScanObjectData(json& object);

private:
	//カメラ
	BaseCamera* camera_ = nullptr;

	//オブジェクトのコンテナ
	std::list<std::unique_ptr<Object3d>> objects_;

	//<!>テクスチャはレベルエディターが対応していないため実装不可

};

