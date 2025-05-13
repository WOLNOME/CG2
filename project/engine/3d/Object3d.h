#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "BaseCamera.h"
#include "MyMath.h"
#include "Model.h"
#include "AnimationModel.h"
#include "Shape.h"
#include "ModelFormat.h"
#include "WorldTransform.h"

//前方宣言
class SceneLight;

//初期化用のタグ
struct ModelTag {};
struct AnimationModelTag {};
struct ShapeTag {};
//モデル
class Object3d {
public://列挙型
	enum class ObjectKind {
		Model,				//通常モデル
		AnimationModel,		//アニメーションモデル
		Shape,				//単純形状

		kMaxNumObjectKind,
	};

public://構造体
	struct FlagForPS {
		uint32_t isActiveLights;
		uint32_t isActiveEnvironment;
	};

public://メンバ関数
	Object3d();

	//モデル初期化
	void Initialize(ModelTag, const std::string& filePath, ModelFormat format = OBJ);
	//アニメーションモデル初期化
	void Initialize(AnimationModelTag, const std::string& filePath, ModelFormat format = GLTF);
	//形状初期化
	void Initialize(ShapeTag, Shape::ShapeKind kind);
	//更新処理
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="_camera">カメラ</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void Draw(const BaseCamera* _camera, int32_t _textureHandle = EOF);

public://setter
	//シーンライト
	void SetSceneLight(SceneLight* _light) { light = _light; }
	//環境光テクスチャ
	void SetEnvironmentLightTextureHandle(int32_t _textureHandle) { environmentLightTextureHandle_ = _textureHandle; }
public: //アニメーション専用の関数
	//新しいアニメーションをセット
	void SetNewAnimation(const std::string& _name, const std::string& _filePath);
	//現在のアニメーションを変更
	void SetCurrentAnimation(const std::string& _name);

private://描画に利用する追加ソース
	//シーンライト
	SceneLight* light;
	//環境光用のテクスチャ
	int32_t environmentLightTextureHandle_ = EOF;
public://外からいじれるメンバ変数
	WorldTransform worldTransform;

private://メンバ変数
	//モデル
	Model* model_ = nullptr;
	//アニメーションモデル
	AnimationModel* animationModel_ = nullptr;
	//形状
	std::unique_ptr<Shape> shape_ = nullptr;

	//オブジェクトの種類
	ObjectKind objKind_;

	//ライト有無用定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> flagResource_;
	//ライト有無用データ
	FlagForPS* flagData_ = nullptr;

};

