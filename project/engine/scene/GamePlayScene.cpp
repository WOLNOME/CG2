#include "GamePlayScene.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "LineDrawerCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"


void GamePlayScene::Initialize()
{
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//カメラの生成と初期化
	camera_ = std::make_unique<GamePlayCamera>();
	camera_->Initialize();
	camera_->SetTranslate({ 0.0f,30.0f,-95.0f });
	camera_->SetRotate({ 0.2f,0.0f,0.0f });
	camera_->SetFarClip(1000.0f);
	
	//光源系の生成と初期化
	dLight_ = std::make_unique<DirectionalLight>();
	dLight_->Initialize();
	dLight_->direction_ = Vector3(1.0f, -1.0f, 0.0f).Normalized();
	//光源をシーンライトにセット
	sceneLight_->SetLight(dLight_.get());

	//天球の生成と初期化
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();
	//地面の生成と初期化
	ground_ = std::make_unique<Ground>();
	ground_->Initialize();
	//移動制限枠系の生成と初期化
	wtFrameObject_.Initialize();
	frameObject_ = std::make_unique<Object3d>();
	frameObject_->InitializeModel("Frame", OBJ);

	//プレイヤーの生成と初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();
	//エネミーの生成と初期化
	enemy_ = std::make_unique<Enemy>();
	enemy_->Initialize();

	//衝突マネージャーの生成
	collisionManager_ = std::make_unique<CollisionManager>();
}

void GamePlayScene::Finalize()
{
}

void GamePlayScene::Update()
{
	//シーン遷移
#ifdef _DEBUG
	if (input_->TriggerKey(DIK_TAB)) {
		sceneManager_->SetNextScene("TITLE");
	}
#endif // _DEBUG
	if (enemy_->GetIsDead()) {
		sceneManager_->SetNextScene("CLEAR");
	}
	
	//カメラの更新
	camera_->Update();
	//ライトの更新
	sceneLight_->Update(camera_.get());

	//天球の更新
	skydome_->Update();
	//地面の更新
	ground_->Update();
	//移動制限枠の更新
	wtFrameObject_.UpdateMatrix();


	//プレイヤーの更新
	player_->Update();
	//エネミーの更新
	enemy_->Update();

	//当たり判定処理
	CheckAllCollision();


#ifdef _DEBUG
	ImGui::Begin("scene");
	ImGui::Text("%s", "GAMEPLAY");
	ImGui::End();
#endif // _DEBUG
}

void GamePlayScene::Draw()
{
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	//天球
	skydome_->Draw(*camera_, nullptr);
	//地面
	ground_->Draw(*camera_, nullptr);
	//移動制限枠
	frameObject_->Draw(wtFrameObject_, *camera_, sceneLight_.get());

	//プレイヤー
	player_->Draw(*camera_, sceneLight_.get());
	//エネミー
	enemy_->Draw(*camera_, sceneLight_.get());

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
	///------------------------------///

	//パーティクルの共通描画設定
	ParticleCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓パーティクル描画開始↓↓↓↓
	///------------------------------///


	///------------------------------///
	///↑↑↑↑パーティクル描画終了↑↑↑↑
	///------------------------------///


	//線描画共通描画設定
	LineDrawerCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓線描画開始↓↓↓↓
	///------------------------------///

	player_->DrawLine(*camera_);
	enemy_->DrawLine(*camera_);

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

void GamePlayScene::CheckAllCollision()
{
	//衝突マネージャーのクリア
	collisionManager_->ClearColliders();

	//コライダー
	std::list<Collider*> colliders;
	//コライダーをリストに登録
	colliders.push_back(player_->GetBullet().get());
	colliders.push_back(enemy_.get());

	//衝突マネージャーのリストにコライダーを登録する
	collisionManager_->SetColliders(colliders);
	//衝突判定の当たり判定処理を呼び出す
	collisionManager_->CheckCollision();
}
