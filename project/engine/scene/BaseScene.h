#pragma once
//シーンの基底クラス
#include "SceneLight.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
//オブジェクトクラス
#include "Sprite.h"
#include "Object3d.h"
#include "Particle.h"
#include "CombinedParticle.h"
#include "ParticleManager.h"
#include "Input.h"
#include "Audio.h"
#include "LineDrawer.h"
#include "TextureManager.h"
#include "TextTextureManager.h"
#include "LevelObject.h"
//シーン共通クラス
#include "PostEffectManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "LineDrawerCommon.h"
#include "SpriteCommon.h"
//シーン共通ヘッダー
#include <cstdint>
#include <chrono>
#include <memory>

class SceneManager;
class BaseScene
{
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~BaseScene();
	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize();
	/// <summary>
	/// 終了時
	/// </summary>
	virtual void Finalize();
	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update();
	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw();
	

	//FPS表示
	void ShowFPS();
	
protected:
	//シーンマネージャー
	SceneManager* sceneManager_ = nullptr;
	//シーンライト
	std::unique_ptr<SceneLight> sceneLight_ = nullptr;

	//fps計測用変数
	std::chrono::steady_clock::time_point lastFrameTime_= std::chrono::steady_clock::now();
	float fps_ = 0.0f;

};

