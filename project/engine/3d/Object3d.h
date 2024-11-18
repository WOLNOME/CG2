#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <memory>
#include "MyMath.h"
#include "Model.h"
#include "WorldTransform.h"
#include "DirectionalLight.h"

class BaseCamera;
//モデル
class Object3d
{
public://構造体
	struct LightFlagForPS
	{
		uint32_t isDirectionalLight;
	};
public://メンバ関数
	//初期化
	void Initialize(const std::string& filePath);
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldTransform">ワールドトランスフォーム</param>
	/// <param name="camera">カメラ</param>
	/// <param name="dirLight">平行光源</param>
	void Draw(const WorldTransform& worldTransform, const BaseCamera& camera, const DirectionalLight* dirLight = nullptr);
private://非公開メンバ関数

private://メンバ変数
	//モデル
	Model* model_;

	//光源有無リソース定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> lightFlagResource_;
	// マッピング済みアドレス
	LightFlagForPS* lightFlagData_ = nullptr;


};

