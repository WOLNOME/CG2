#include "EvaluationTaskScene.h"
#include "SceneManager.h"
#include <numbers>

void EvaluationTaskScene::Initialize() {
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//カメラの生成と初期化
	camera = std::make_unique<DevelopCamera>();
	camera->Initialize();
	camera->SetRotate({ cameraRotate });
	camera->SetTranslate(cameraTranslate);
	camera->SetFarClip(500.0f);

	//スカイボックスの生成と初期化
	textureHandleSkyBox_ = TextureManager::GetInstance()->LoadTexture("earth-cubemap.dds");
	skyBox_ = std::make_unique<Object3d>();
	skyBox_->Initialize(ShapeTag{}, Shape::ShapeKind::kSkyBox);
	skyBox_->worldTransform.scale = { 300.0f,300.0f,300.0f };

	ParticleManager::GetInstance()->SetCamera(camera.get());
	//爆発エフェクトアニメーション
	explosionEffects_.resize((int)ExplosionEffectName::kMaxNumExplosionEffectName);
	{
		//収束のエフェクト
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_Convergence"), "convergence");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 0.1f,0.1f,0.1f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Clump;
			particle->emitter_.clumpNum = 3;
			particle->emitter_.effectStyle = Particle::EffectStyle::OneShot;
			particle->emitter_.isBillboard = false;
			float startTime = 1.0f / 60.0f;
			float endTime = 0.0f;
			//該当のインデックスに追加
			explosionEffects_[(int)ExplosionEffectName::Convergence] = { std::move(particle), startTime, endTime };
		}
		//閃光のエフェクト
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_Flush"), "flash");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 0.1f,0.1f,0.1f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Clump;
			particle->emitter_.clumpNum = 3;
			particle->emitter_.effectStyle = Particle::EffectStyle::OneShot;
			float startTime = 6.0f/60.0f;
			float endTime = 0.0f;
			//該当のインデックスに追加
			explosionEffects_[(int)ExplosionEffectName::Flash] = { std::move(particle), startTime, endTime };
		}
		//衝撃波のエフェクト
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_ShockWave"), "shockwave");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 0.1f,0.1f,0.1f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Random;
			particle->emitter_.effectStyle = Particle::EffectStyle::Loop;
			particle->emitter_.isBillboard = false;
			float startTime = 12.0f/60.0f;
			float endTime = 40.0f/60.0f;
			//該当のインデックスに追加
			explosionEffects_[(int)ExplosionEffectName::ShockWave] = { std::move(particle), startTime, endTime };
		}
		//炎のエフェクト
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_Fire"), "fire");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 0.1f,0.1f,0.1f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Clump;
			particle->emitter_.clumpNum = 3;
			particle->emitter_.effectStyle = Particle::EffectStyle::Loop;
			particle->emitter_.isGravity = true;
			particle->emitter_.gravity = 2.0f;
			float startTime = 12.0f/60.0f;
			float endTime = 40.0f/60.0f;
			//該当のインデックスに追加
			explosionEffects_[(int)ExplosionEffectName::Fire] = { std::move(particle), startTime, endTime };
		}
		//煙のエフェクト
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_ShockWave"), "smoke");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 12.5f,12.5f,12.5f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Clump;
			particle->emitter_.clumpNum = 8;
			particle->emitter_.effectStyle = Particle::EffectStyle::Loop;
			particle->emitter_.isGravity = true;
			particle->emitter_.gravity = 6.0f;
			float startTime = 30.0f/60.0f;
			float endTime = 60.0f/60.0f;
			//該当のインデックスに追加
			explosionEffects_[(int)ExplosionEffectName::Smoke] = { std::move(particle), startTime, endTime };
		}
		//瓦礫エフェクト
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_Rubble"), "rubble");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 5.0f,5.0f,5.0f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Random;
			particle->emitter_.effectStyle = Particle::EffectStyle::Loop;
			particle->emitter_.isGravity = true;
			particle->emitter_.gravity = -40.0f;
			float startTime = 20.0f / 60.0f;
			float endTime = 50.0f / 60.0f;
			//該当のインデックスに追加
			explosionEffects_[(int)ExplosionEffectName::Rubble] = { std::move(particle), startTime, endTime };
		}
	}
	//闇エフェクト
	darknessEffects_.resize((int)DarknessEffectName::kMaxNumDarknessEffectName);
	{
		//うずまきエフェクト
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_Uzumaki"), "uzumaki");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 0.1f,0.1f,0.1f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Clump;
			particle->emitter_.clumpNum = 6;
			particle->emitter_.effectStyle = Particle::EffectStyle::OneShot;
			float startTime = 0.01f / 60.0f;
			float endTime = 0.0f / 60.0f;
			//該当のインデックスに追加
			darknessEffects_[(int)DarknessEffectName::Uzumaki] = { std::move(particle), startTime, endTime };
		}
		//暗粒
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_DarkGrain"), "darkgrain");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 3.0f,3.0f,3.0f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Random;
			particle->emitter_.effectStyle = Particle::EffectStyle::Loop;
			float startTime = 0.01f / 60.0f;
			float endTime = 40.0f / 60.0f;
			//該当のインデックスに追加
			darknessEffects_[(int)DarknessEffectName::DarkGrain] = { std::move(particle), startTime, endTime };
		}
		//閃光
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_DarkFlash"), "darkflash");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 0.1f,0.1f,0.1f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Clump;
			particle->emitter_.clumpNum = 4;
			particle->emitter_.effectStyle = Particle::EffectStyle::OneShot;
			float startTime = 0.01f / 60.0f;
			float endTime = 0.0f / 60.0f;
			//該当のインデックスに追加
			darknessEffects_[(int)DarknessEffectName::Flash] = { std::move(particle), startTime, endTime };
		}
		//集中
		{
			//生成と初期化
			std::unique_ptr<Particle> particle = std::make_unique<Particle>();
			particle->Initialize(ParticleManager::GetInstance()->GenerateName("Explosion_Concentration"), "concentration");
			particle->emitter_.isPlay = false;
			particle->emitter_.transform.scale = { 0.1f,0.1f,0.1f };
			particle->emitter_.generateMethod = Particle::GenerateMethod::Clump;
			particle->emitter_.clumpNum = 2;
			particle->emitter_.effectStyle = Particle::EffectStyle::Loop;
			float startTime = 1.0f / 60.0f;
			float endTime = 40.0f / 60.0f;
			//該当のインデックスに追加
			darknessEffects_[(int)DarknessEffectName::Concentration] = { std::move(particle), startTime, endTime };
		}

	}



}

void EvaluationTaskScene::Finalize() {
}

void EvaluationTaskScene::Update() {
	//シーン共通の更新
	BaseScene::Update();

	//TABでシーン再読み込み
	if (input_->TriggerKey(DIK_TAB)) {
		sceneManager_->SetNextScene("EVALUATIONTASK");
	}

	//カメラの更新
	camera->Update();

	//スカイボックスの更新
	skyBox_->Update();

	//エフェクトの更新
	ExplosionEffectUpdate();
	DarknessEffectUpdate();

#ifdef _DEBUG

	ImGui::Begin("アニメーションの再生管理");
	//エフェクト
	{
		//爆発エフェクト再生
		if (ImGui::Button("爆発エフェクト再生")) {
			if (isShake_&&!isExplosionPlay_) {
				camera->RegistShake(3.0f, 0.7f);
			}
			isExplosionPlay_ = true;
			camera->SetTranslate({ 0.0f,0.0f,-230.0f });
		}
		//闇エフェクト再生
		if (ImGui::Button("闇攻撃エフェクト再生")) {
			if (isShake_ && !isDarknessPlay_) {
				camera->RegistShake(2.0f, 0.3f);
			}
			isDarknessPlay_ = true;
			camera->SetTranslate({ 0.0f,0.0f,-35.0f });
		}

		//揺れフラグを追加
		ImGui::Checkbox("ゆらす", &isShake_);
	}
	ImGui::End();
#endif // _DEBUG
}

void EvaluationTaskScene::Draw() {
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	//スカイボックス描画
	skyBox_->Draw(camera.get(), textureHandleSkyBox_);

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
	///------------------------------///


	//線描画共通描画設定
	LineDrawerCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓線描画開始↓↓↓↓
	///------------------------------///

	///------------------------------///
	///↑↑↑↑線描画終了↑↑↑↑
	///------------------------------///

	//スプライトの共通描画設定
	SpriteCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓スプライト描画開始↓↓↓↓
	///------------------------------///


	///------------------------------///
	///↑↑↑↑スプライト描画終了↑↑↑↑
	///------------------------------///
}

void EvaluationTaskScene::TextDraw() {
	///------------------------------///
	///↑↑↑↑テキスト描画終了↑↑↑↑
	///------------------------------///

	///------------------------------///
	///↑↑↑↑テキスト描画終了↑↑↑↑
	///------------------------------///
}

void EvaluationTaskScene::ExplosionEffectUpdate() {
	//もし再生フラグがオンになったら
	if (isExplosionPlay_) {
		explosionCurrentTime_ += kDeltaTime;
		//エフェクトを回す
		for (auto& effect : explosionEffects_) {
			//もしスタイルがOneShotなら
			if (effect.particle->emitter_.effectStyle == Particle::EffectStyle::OneShot) {
				//開始時間に達したら
				if (explosionCurrentTime_ > effect.startTime && explosionCurrentTime_ - kDeltaTime <= effect.startTime) {
					effect.particle->emitter_.isPlay = true;
				}
			}
			//もしスタイルがLoopなら
			else if (effect.particle->emitter_.effectStyle == Particle::EffectStyle::Loop) {
				//開始時間に達したら
				if (explosionCurrentTime_ > effect.startTime && explosionCurrentTime_ - kDeltaTime <= effect.startTime) {
					effect.particle->emitter_.isPlay = true;
				}
				//終了時間に達したら
				if (explosionCurrentTime_ > effect.endTime && explosionCurrentTime_ - kDeltaTime <= effect.endTime) {
					effect.particle->emitter_.isPlay = false;
				}
			}
		}
		//3.2秒たったら終了
		if (explosionCurrentTime_ > 3.2f) {
			isExplosionPlay_ = false;
			explosionCurrentTime_ = 0.0f;
		}
	}
}

void EvaluationTaskScene::DarknessEffectUpdate() {
	//もし再生フラグがオンになったら
	if (isDarknessPlay_) {
		darknessCurrentTime_ += kDeltaTime;
		//エフェクトを回す
		for (auto& effect : darknessEffects_) {
			//もしスタイルがOneShotなら
			if (effect.particle->emitter_.effectStyle == Particle::EffectStyle::OneShot) {
				//開始時間に達したら
				if (darknessCurrentTime_ > effect.startTime && darknessCurrentTime_ - kDeltaTime <= effect.startTime) {
					effect.particle->emitter_.isPlay = true;
				}
			}
			//もしスタイルがLoopなら
			else if (effect.particle->emitter_.effectStyle == Particle::EffectStyle::Loop) {
				//開始時間に達したら
				if (darknessCurrentTime_ > effect.startTime && darknessCurrentTime_ - kDeltaTime <= effect.startTime) {
					effect.particle->emitter_.isPlay = true;
				}
				//終了時間に達したら
				if (darknessCurrentTime_ > effect.endTime && darknessCurrentTime_ - kDeltaTime <= effect.endTime) {
					effect.particle->emitter_.isPlay = false;
				}
			}
		}
		//3.2秒たったら終了
		if (darknessCurrentTime_ > 3.2f) {
			isDarknessPlay_ = false;
			darknessCurrentTime_ = 0.0f;
		}
	}
}
