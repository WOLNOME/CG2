#include "DevelopScene.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "LineDrawerCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"

void DevelopScene::Initialize()
{
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//カメラの生成と初期化
	camera = std::make_unique<DevelopCamera>();
	camera->Initialize();
	camera->SetRotate({ cameraRotate });
	camera->SetTranslate(cameraTranslate);

	//平行光源の生成と初期化
	dirLight = std::make_unique<DirectionalLight>();
	dirLight->Initialize();
	//点光源の生成と初期化
	pointLight = std::make_unique<PointLight>();
	pointLight->Initialize();
	//点光源目印の生成と初期化
	plMark = std::make_unique<LineDrawer>();
	plMark->Initialize();


	//ゲームシーン変数の初期化
	sprite_ = std::make_unique<Sprite>();
	textureHandleSprite_ = TextureManager::GetInstance()->LoadTexture("monsterBall.png");
	sprite_->Initialize(textureHandleSprite_);
	sprite_->SetAnchorPoint({ 0.5f,0.5f });
	sprite_->SetFlipX(true);

	sprite2_ = std::make_unique<Sprite>();
	textureHandleSprite2_ = TextureManager::GetInstance()->LoadTexture("monsterBall.png");
	sprite2_->Initialize(textureHandleSprite2_);
	sprite2Position = { 100.0f,100.0f };
	sprite2_->SetPosition(sprite2Position);
	sprite2_->SetSize({ 300.0f,300.0f });

	wtAxis_.Initialize();
	axis_ = std::make_unique<Object3d>();
	axis_->Initialize("teapot");

	wtTerrain_.Initialize();
	wtTerrain_.translate_ = { 0.0f,-1.2f,0.0f };
	terrain_ = std::make_unique<Object3d>();
	terrain_->Initialize("terrain");

	particle_ = std::make_unique<Particle>();
	particle_->Initialize("plane");

	line_ = std::make_unique<LineDrawer>();
	line_->Initialize();

	audio_ = std::make_unique<Audio>();
	audio_->Initialize("Alarm01.wav");
}

void DevelopScene::Finalize()
{
}

void DevelopScene::Update()
{
	//カメラの更新
	camera->Update();

	//平行光源の更新
	dirLight->Update();

	//点光源の更新
	pointLight->Update();

	//モデルの更新
	wtAxis_.rotate_.y += 0.03f;
	wtAxis_.UpdateMatrix();
	wtTerrain_.UpdateMatrix();


	//パーティクル
	particle_->Update();

	//スプライトの更新
	sprite_->Update();
	sprite_->SetRotation(sprite_->GetRotation() + 0.03f);
	sprite2_->Update();

#ifdef _DEBUG
	ImGui::SetNextWindowSize(ImVec2(500, 100));
	ImGui::Begin("MosterBall");
	ImGui::SliderFloat2("position", &sprite2Position.x, 0.0f, 1200.0f, "%5.1f");
	sprite2_->SetPosition(sprite2Position);
	ImGui::End();

	ImGui::Begin("Audio");
	if (ImGui::Button("PlayAudio")) {
		audio_->Play();
	}
	if (ImGui::Button("StopAudio")) {
		audio_->Stop();
	}
	if (ImGui::Button("PauseAudio")) {
		audio_->Pause();
	}
	if (ImGui::Button("ResumeAudio")) {
		audio_->Resume();
	}
	ImGui::SliderFloat("SetVolume", &volume, 0.0f, 1.0f);
	audio_->SetVolume(volume);

	ImGui::End();

	ImGui::Begin("line");
	ImGui::Checkbox("sphere", &isDrawSphere_);
	if (isDrawSphere_) {
		Sphere sphere;
		sphere.center = { 0.0f,-3.0f,0.0f };
		sphere.radius = 2.0f;
		MyMath::DrawSphere(sphere, { 1.0f,0.0f,0.0f,1.0f }, line_.get());
	}

	ImGui::End();

	ImGui::Begin("teapot");
	ImGui::DragFloat3("translate", &wtAxis_.translate_.x, 0.01f);
	ImGui::DragFloat3("scale", &wtAxis_.scale_.x, 0.01f);
	ImGui::End();

	ImGui::Begin("terrain");
	ImGui::DragFloat3("translate", &wtTerrain_.translate_.x, 0.01f);
	ImGui::DragFloat3("scale", &wtTerrain_.scale_.x, 0.01f);
	ImGui::End();

	ImGui::Begin("DirectionalLight");
	ImGui::DragFloat4("color", &dirLight->color_.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat3("direction", &dirLight->direction_.x, 0.01f);
	ImGui::DragFloat("intencity", &dirLight->intencity_, 0.01f, 0.0f, 1.0f);
	ImGui::Checkbox("isActive", &dirLight->isActive_);
	ImGui::End();

	ImGui::Begin("PoiintLight");
	ImGui::DragFloat4("color", &pointLight->color_.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat3("position", &pointLight->position_.x, 0.01f);
	ImGui::DragFloat("intencity", &pointLight->intencity_, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("radius", &pointLight->radius_, 0.01f, 0.0f, 20.0f);
	ImGui::DragFloat("decay", &pointLight->decay_, 0.01f, 0.0f, 10.0f);
	ImGui::Checkbox("isActive", &pointLight->isActive_);
	ImGui::Checkbox("isDrawMark", &isDrawMark);
	if (isDrawMark) {
		//マークの生成
		Sphere plMarkSphere;
		plMarkSphere.center = pointLight->position_;
		plMarkSphere.radius = 0.1f;
		MyMath::DrawSphere(plMarkSphere, { 1.0f,0.5f,0.0f,1.0f }, plMark.get());

	}
	ImGui::End();

#endif // _DEBUG
}

void DevelopScene::Draw()
{
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	axis_->Draw(wtAxis_, *camera.get(), dirLight.get(),pointLight.get());

	terrain_->Draw(wtTerrain_, *camera.get(), dirLight.get(), pointLight.get());

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
	///------------------------------///

	//パーティクルの共通描画設定
	ParticleCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓パーティクル描画開始↓↓↓↓
	///------------------------------///

	particle_->Draw(*camera.get());

	///------------------------------///
	///↑↑↑↑パーティクル描画終了↑↑↑↑
	///------------------------------///


	//線描画共通描画設定
	LineDrawerCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓線描画開始↓↓↓↓
	///------------------------------///

	//線描画
	line_->Draw(*camera.get());
	plMark->Draw(*camera.get());

	///------------------------------///
	///↑↑↑↑線描画終了↑↑↑↑
	///------------------------------///

	//スプライトの共通描画設定
	SpriteCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓スプライト描画開始↓↓↓↓
	///------------------------------///

	//スプライト描画
	sprite_->Draw();
	sprite2_->Draw();


	///------------------------------///
	///↑↑↑↑スプライト描画終了↑↑↑↑
	///------------------------------///
}
