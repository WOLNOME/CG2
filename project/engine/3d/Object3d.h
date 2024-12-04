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
#include "Model.h"

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
		//座標変換行列データ
		struct TransformationMatrix {
			Matrix4x4 WVP;
			Matrix4x4 World;
		};
		//Object3d用リソース作成用データ型
		struct Object3dResource {
			Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
			TransformationMatrix* wvpData;
			Transform transform;
		};
	};
public://メンバ関数
	//初期化
	void Initialize(const std::string& filePath);
	void Update();
	void Draw();
private://非公開メンバ関数
	//Object3dリソース作成関数
	Struct::Object3dResource MakeObject3dResource();
public://ゲッター
	const Vector3& GetScale() { return object3dResource_.transform.scale; }
	const Vector3& GetRotate() { return object3dResource_.transform.rotate; }
	const Vector3& GetTranslate() { return object3dResource_.transform.translate; }
public://セッター
	void SetScale(const Vector3& scale) { object3dResource_.transform.scale = scale; }
	void SetRotate(const Vector3& rotate) { object3dResource_.transform.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { object3dResource_.transform.translate = translate; }
	
private://メンバ変数
	//モデル
	Model* model_;
	//Object3d用リソース
	Struct::Object3dResource object3dResource_;

	//平行光源用バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;
	//平行光源用バッファリソース内のデータをさすポインタ
	Struct::DirectionalLight* directionalLightData = nullptr;


};

