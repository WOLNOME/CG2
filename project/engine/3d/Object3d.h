#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <memory>
#include "MyMath.h"
#include "Model.h"
#include "WorldTransform.h"
#include "Camera.h"

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
	void Initialize(const std::string& filePath);
	void Draw(const WorldTransform& worldTransform,Camera* camera);
private://非公開メンバ関数

private://メンバ変数
	//モデル
	Model* model_;
	

	//平行光源用バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	//平行光源用バッファリソース内のデータをさすポインタ
	Struct::DirectionalLight* directionalLightData = nullptr;


};

