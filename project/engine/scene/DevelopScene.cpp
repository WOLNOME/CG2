#include "DevelopScene.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "LineDrawerCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"
#include <numbers>

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
	camera->SetFarClip(100.0f);

	//平行光源の生成と初期化
	dirLight = std::make_unique<DirectionalLight>();
	//点光源の生成と初期化
	pointLight = std::make_unique<PointLight>();
	pointLight2 = std::make_unique<PointLight>();
	//点光源目印の生成と初期化
	plMark = std::make_unique<LineDrawer>();
	plMark->Initialize();
	plMark2 = std::make_unique<LineDrawer>();
	plMark2->Initialize();
	//スポットライトの生成と初期化
	spotLight = std::make_unique<SpotLight>();
	//スポットライト目印の生成と初期化
	slMark = std::make_unique<LineDrawer>();
	slMark->Initialize();

	//各光源をシーンライトにセット
	sceneLight_->SetLight(dirLight.get());
	sceneLight_->SetLight(pointLight.get());
	sceneLight_->SetLight(pointLight2.get());
	sceneLight_->SetLight(spotLight.get());


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

	wtAnimatedCube_.Initialize();
	wtAnimatedCube_.translate_ = { 0.0f,3.0f,0.0f };
	animatedCube_ = std::make_unique<Object3d>();
	animatedCube_->Initialize("AnimatedCube", GLTF);

	wtSneakWalk_.Initialize();
	wtSneakWalk_.translate_ = { 3.0f,3.0f,0.0f };
	sneakWalk_ = std::make_unique<Object3d>();
	sneakWalk_->Initialize("sneakWalk", GLTF);

	wtWalk_.Initialize();
	wtWalk_.translate_ = { 4.0f,3.0f,0.0f };
	walk_ = std::make_unique<Object3d>();
	walk_->Initialize("walk", GLTF);

	wtSimpleSkin_.Initialize();
	wtSimpleSkin_.translate_ = { 5.0f,3.0f,0.0f };
	simpleSkin_ = std::make_unique<Object3d>();
	simpleSkin_->Initialize("simpleSkin", GLTF);

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

	//シーンライトの更新処理
	sceneLight_->Update(camera.get());


	//モデルの更新
	wtAxis_.rotate_.y += 0.03f;
	wtAxis_.UpdateMatrix();
	wtTerrain_.UpdateMatrix();
	wtAnimatedCube_.UpdateMatrix();
	wtSneakWalk_.UpdateMatrix();
	wtWalk_.UpdateMatrix();
	wtSimpleSkin_.UpdateMatrix();

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
	ImGui::SliderFloat4("color", &dirLight->color_.x, 0.0f, 1.0f);
	ImGui::DragFloat3("direction", &dirLight->direction_.x, 0.01f);
	ImGui::SliderFloat("intencity", &dirLight->intencity_, 0.0f, 10.0f);
	ImGui::Checkbox("isActive", &dirLight->isActive_);
	ImGui::End();

	ImGui::Begin("PoiintLight2");
	ImGui::SliderFloat4("color", &pointLight2->color_.x, 0.0f, 1.0f);
	ImGui::DragFloat3("position", &pointLight2->position_.x, 0.01f);
	ImGui::SliderFloat("intencity", &pointLight2->intencity_, 0.0f, 10.0f);
	ImGui::SliderFloat("radius", &pointLight2->radius_, 0.0f, 20.0f);
	ImGui::SliderFloat("decay", &pointLight2->decay_, 0.0f, 10.0f);
	ImGui::Checkbox("isActive", &pointLight2->isActive_);
	ImGui::Checkbox("isDrawMark", &isDrawPLMark2);
	if (isDrawPLMark2) {
		//マークの生成
		Sphere plMarkSphere;
		plMarkSphere.center = pointLight2->position_;
		plMarkSphere.radius = 0.1f;
		MyMath::DrawSphere(plMarkSphere, { 1.0f,0.5f,0.0f,1.0f }, plMark2.get());

	}
	ImGui::End();

	ImGui::Begin("PoiintLight");
	ImGui::SliderFloat4("color", &pointLight->color_.x, 0.0f, 1.0f);
	ImGui::DragFloat3("position", &pointLight->position_.x, 0.01f);
	ImGui::SliderFloat("intencity", &pointLight->intencity_, 0.0f, 10.0f);
	ImGui::SliderFloat("radius", &pointLight->radius_, 0.0f, 20.0f);
	ImGui::SliderFloat("decay", &pointLight->decay_, 0.0f, 10.0f);
	ImGui::Checkbox("isActive", &pointLight->isActive_);
	ImGui::Checkbox("isDrawMark", &isDrawPLMark);
	if (isDrawPLMark) {
		//マークの生成
		Sphere plMarkSphere;
		plMarkSphere.center = pointLight->position_;
		plMarkSphere.radius = 0.1f;
		MyMath::DrawSphere(plMarkSphere, { 1.0f,0.5f,0.0f,1.0f }, plMark.get());

	}
	ImGui::End();

	ImGui::Begin("SpotLight");
	ImGui::SliderFloat4("color", &spotLight->color_.x, 0.0f, 1.0f);
	ImGui::DragFloat3("position", &spotLight->position_.x, 0.01f);
	ImGui::SliderFloat("intencity", &spotLight->intencity_, 0.0f, 10.0f);
	ImGui::SliderFloat3("direction", &spotLight->direction_.x, -1.0f, 1.0f);
	ImGui::SliderFloat("distance", &spotLight->distance_, 0.0f, 20.0f);
	ImGui::SliderFloat("decay", &spotLight->decay_, 0.0f, 10.0f);
	ImGui::SliderFloat("cosAngle", &spotLight->cosAngle_, -1.0f, spotLight->cosFalloffStart_ - 0.01f);
	ImGui::SliderFloat("cosFalloffStart", &spotLight->cosFalloffStart_, 0.0f, 2.0f);
	ImGui::Checkbox("isActive", &spotLight->isActive_);
	ImGui::Checkbox("isDrawMark", &isDrawSLMark);
	if (isDrawSLMark) {
		//マークの生成
		Sphere slMarkSphere;
		slMarkSphere.center = spotLight->position_;
		slMarkSphere.radius = 0.1f;
		Sphere slMarkSphere2;
		slMarkSphere2.center = spotLight->position_ + (spotLight->direction_.Normalized() * 0.15f);
		slMarkSphere2.radius = 0.05f;

		MyMath::DrawSphere(slMarkSphere, { 1.0f,0.25f,0.0f,1.0f }, slMark.get());
		MyMath::DrawSphere(slMarkSphere2, { 0.0f,1.0f,0.0f,1.0f }, slMark.get());
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

	axis_->Draw(wtAxis_, *camera.get(), sceneLight_.get());

	terrain_->Draw(wtTerrain_, *camera.get(), sceneLight_.get());

	animatedCube_->Draw(wtAnimatedCube_, *camera.get(), sceneLight_.get());

	sneakWalk_->Draw(wtSneakWalk_, *camera.get(), sceneLight_.get());

	walk_->Draw(wtWalk_, *camera.get(), sceneLight_.get());

	simpleSkin_->Draw(wtSimpleSkin_, *camera.get(), sceneLight_.get());

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
	plMark2->Draw(*camera.get());
	slMark->Draw(*camera.get());

	//オブジェクトのボーン線描画
	animatedCube_->DrawLine(wtAnimatedCube_, *camera.get());
	sneakWalk_->DrawLine(wtSneakWalk_, *camera.get());
	walk_->DrawLine(wtWalk_, *camera.get());
	simpleSkin_->DrawLine(wtSimpleSkin_, *camera.get());

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
