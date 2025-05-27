#pragma once
#include "BaseScene.h"
#include "Vector2.h"
#include "MyMath.h"
#include "DevelopCamera.h"
#include "MyMath.h"
#include <vector>

class EvaluationTaskScene : public BaseScene {
private://列挙体
	//爆発パーティクル
	enum class ExplosionParticleName{
		Convergence,	//収束
		Flash,			//閃光
		ShockWave,		//衝撃波
		Fire,			//炎
		Smoke,			//煙

		kMaxNumExplosionParticleName,	//最大数
	};
private://構造体
	//単体パーティクルのデータ
	struct SingleEffectData {
		std::unique_ptr<Particle> particle;	//パーティクル
		float startTime;				//開始時間
		float endTime;					//終了時間
	};

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// 終了時
	/// </summary>
	void Finalize() override;
	/// <summary>
	/// 更新
	/// </summary>
	void Update() override;
	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;
	/// <summary>
	///	テキスト描画
	/// </summary>
	void TextDraw() override;

private:
	//パーティクルの更新
	void EffectUpdate();

private://メンバ変数
	Input* input_ = nullptr;
	//開発用カメラ
	std::unique_ptr<DevelopCamera> camera;
	Vector3 cameraTranslate = { 0.0f,0.0f,-15.0f };
	Vector3 cameraRotate = { 0.0f,0.0f,0.0f };
	//スカイボックス
	uint32_t textureHandleSkyBox_ = 0u;
	std::unique_ptr<Object3d> skyBox_ = nullptr;

	//パーティクル
	std::vector<SingleEffectData> explosionEffects_;
	bool isExplosionPlay_ = false;	//再生フラグ
	float currentTime_ = 0.0f;

};

