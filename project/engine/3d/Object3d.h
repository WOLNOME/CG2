#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "MyMath.h"
#include "Model.h"
#include "AnimationModel.h"
#include "Shape.h"
#include "ModelFormat.h"
#include "WorldTransform.h"
#include "SceneLight.h"

class BaseCamera;
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
	/// <param name="camera">カメラ</param>
	/// <param name="dirLight">シーン内光源</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	void Draw(
		const BaseCamera& camera,
		const SceneLight* sceneLight = nullptr,
		int32_t textureHandle = EOF
	);

public://setter
	void SetEnvironmentLightTextureHandle(int32_t _textureHandle) { environmentLightTextureHandle_ = _textureHandle; }

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

	//環境光用のテクスチャ
	int32_t environmentLightTextureHandle_ = EOF;

};

