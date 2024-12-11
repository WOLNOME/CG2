#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "MyMath.h"
#include "Model.h"
#include "ModelFormat.h"
#include "WorldTransform.h"
#include "SceneLight.h"

class BaseCamera;
//モデル
class Object3d
{
public://メンバ関数
	//初期化
	void Initialize(const std::string& filePath, ModelFormat format = OBJ);
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldTransform">ワールドトランスフォーム</param>
	/// <param name="camera">カメラ</param>
	/// <param name="dirLight">シーン内光源</param>
	void Draw(
		WorldTransform& worldTransform,
		const BaseCamera& camera,
		const SceneLight* sceneLight = nullptr
	);
	
	void DrawLine(
		const WorldTransform& worldTransform,
		const BaseCamera& camera
	);

private://非公開メンバ関数

private://メンバ変数
	//モデル
	Model* model_;

};

