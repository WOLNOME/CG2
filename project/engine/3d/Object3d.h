#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <memory>
#include "Function.h"
#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class Object3dCommon;
class Model;
//モデル
class Object3d
{
public://インナークラス
	class Struct {
	public:
		//平行光源データ
		struct DirectionalLight
		{
			Vector4 color;
			Vector3 direction;
			float intensity;
		};
	};
public://メンバ関数
	//初期化
	void Initialize(Object3dCommon* modelCommon);
	void Update();
	void Draw();
public://ゲッター
	Model* GetModel() { return model_.get(); }
public://セッター
	void SetModel(const std::string& filePath);

private://インスタンス
	Object3dCommon* object3dCommon_ = nullptr;
	std::unique_ptr<Model> model_;
private://メンバ変数

	//平行光源用バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	//平行光源用バッファリソース内のデータをさすポインタ
	Struct::DirectionalLight* directionalLightData = nullptr;


};

