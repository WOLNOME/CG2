#include "DevelopScene.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "Object3dCommon.h"
#include "ParticleCommon.h"
#include "LineDrawerCommon.h"
#include "SpriteCommon.h"
#include "SceneManager.h"
#include <numbers>
#include <random>

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
	dirLight_ = std::make_unique<DirectionalLight>();
	//点光源の生成と初期化
	pointLight1_ = std::make_unique<PointLight>();
	pointLight2_ = std::make_unique<PointLight>();
	//スポットライトの生成と初期化
	spotLight1_ = std::make_unique<SpotLight>();
	spotLight2_ = std::make_unique<SpotLight>();
	
	//各光源をシーンライトにセット
	sceneLight_->SetLight(dirLight_.get());
	sceneLight_->SetLight(pointLight1_.get());
	sceneLight_->SetLight(pointLight2_.get());
	sceneLight_->SetLight(spotLight1_.get());
	sceneLight_->SetLight(spotLight2_.get());

	//ポイントライトの速度決定
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());
	std::uniform_real_distribution<float> distribution(-0.1f, 0.1f);
	pl1Velocity_ = { distribution(randomEngine),distribution(randomEngine), distribution(randomEngine) };
	pl2Velocity_ = { distribution(randomEngine),distribution(randomEngine), distribution(randomEngine) };

	//スポットライトの位置と向きを固定
	spotLight1_->position_ = { -1.8f,-0.99f,-1.32f };
	spotLight1_->direction_ = { 1.0f,-0.349f,1.0f };
	spotLight2_->position_ = { 1.84f,-0.78f,0.0f };
	spotLight2_->direction_ = { -1.0f,1.0f,0.086f };

	//ゲームシーン変数の初期化
	wtSphere_.Initialize();
	thSphere_ = TextureManager::GetInstance()->LoadTexture("monsterBall.png");
	sphere_ = std::make_unique<Object3d>();
	sphere_->InitializeShape(Shape::kSphere);

	wtPlaneObj_.Initialize();
	wtPlaneObj_.translate_ = { -4.86f,1.66f,0.0f };
	wtPlaneObj_.rotate_ = { 0.0f,3.14f,0.0f };
	planeObj_ = std::make_unique<Object3d>();
	planeObj_->InitializeModel("plane", OBJ);

	wtPlaneGltf_.Initialize();
	wtPlaneGltf_.translate_ = { -2.67f,1.66f,0.0f };
	wtPlaneGltf_.rotate_ = { 0.0f,3.14f,0.0f };
	planeGltf_ = std::make_unique<Object3d>();
	planeGltf_->InitializeModel("plane", GLTF);


	wtTerrain_.Initialize();
	wtTerrain_.translate_ = { 0.0f,-1.2f,0.0f };
	terrain_ = std::make_unique<Object3d>();
	terrain_->InitializeModel("terrain");

	wtAnimatedCube_.Initialize();
	wtAnimatedCube_.translate_ = { 0.0f,3.0f,0.0f };
	animatedCube_ = std::make_unique<Object3d>();
	animatedCube_->InitializeModel("AnimatedCube", GLTF);

	wtSneakWalk_.Initialize();
	wtSneakWalk_.translate_ = { 3.0f,3.0f,0.0f };
	sneakWalk_ = std::make_unique<Object3d>();
	sneakWalk_->InitializeModel("sneakWalk", GLTF);

	wtWalk_.Initialize();
	wtWalk_.translate_ = { 4.0f,3.0f,0.0f };
	walk_ = std::make_unique<Object3d>();
	walk_->InitializeModel("walk", GLTF);

	wtSimpleSkin_.Initialize();
	wtSimpleSkin_.translate_ = { 5.0f,3.0f,0.0f };
	simpleSkin_ = std::make_unique<Object3d>();
	simpleSkin_->InitializeModel("simpleSkin", GLTF);

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
	wtSphere_.rotate_.y += 0.03f;
	wtSphere_.UpdateMatrix();
	wtPlaneObj_.UpdateMatrix();
	wtPlaneGltf_.UpdateMatrix();
	wtTerrain_.UpdateMatrix();
	wtAnimatedCube_.UpdateMatrix();
	wtSneakWalk_.UpdateMatrix();
	wtWalk_.UpdateMatrix();
	wtSimpleSkin_.UpdateMatrix();

#ifdef _DEBUG

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

	ImGui::Begin("sphere");
	ImGui::DragFloat3("translate", &wtSphere_.translate_.x, 0.01f);
	ImGui::DragFloat3("rotate", &wtSphere_.rotate_.x, 0.01f);
	ImGui::DragFloat3("scale", &wtSphere_.scale_.x, 0.01f);
	ImGui::End();

	ImGui::Begin("planeObj");
	ImGui::DragFloat3("translate", &wtPlaneObj_.translate_.x, 0.01f);
	ImGui::DragFloat3("rotate", &wtPlaneObj_.rotate_.x, 0.01f);
	ImGui::DragFloat3("scale", &wtPlaneObj_.scale_.x, 0.01f);
	ImGui::End();

	ImGui::Begin("planeGltf");
	ImGui::DragFloat3("translate", &wtPlaneGltf_.translate_.x, 0.01f);
	ImGui::DragFloat3("rotate", &wtPlaneGltf_.rotate_.x, 0.01f);
	ImGui::DragFloat3("scale", &wtPlaneGltf_.scale_.x, 0.01f);
	ImGui::End();

	ImGui::Begin("terrain");
	ImGui::DragFloat3("translate", &wtTerrain_.translate_.x, 0.01f);
	ImGui::DragFloat3("scale", &wtTerrain_.scale_.x, 0.01f);
	ImGui::End();

	ImGui::Begin("DirectionalLight");
	ImGui::SliderFloat4("color", &dirLight_->color_.x, 0.0f, 1.0f);
	ImGui::DragFloat3("direction", &dirLight_->direction_.x, 0.01f);
	ImGui::SliderFloat("intencity", &dirLight_->intencity_, 0.0f, 10.0f);
	ImGui::Checkbox("isActive", &dirLight_->isActive_);
	ImGui::End();
	//点光源の移動処理
	pointLight1_->position_ += pl1Velocity_;
	pointLight2_->position_ += pl2Velocity_;
	if (pointLight1_->position_.x > 4.0f || pointLight1_->position_.x < -4.0f) {
		pl1Velocity_.x *= -1.0f;
	}
	if (pointLight1_->position_.y > 4.0f || pointLight1_->position_.y < 0.0f) {
		pl1Velocity_.y *= -1.0f;
	}
	if (pointLight1_->position_.z > 4.0f || pointLight1_->position_.z < -4.0f) {
		pl1Velocity_.z *= -1.0f;
	}
	if (pointLight2_->position_.x > 4.0f || pointLight2_->position_.x < -4.0f) {
		pl2Velocity_.x *= -1.0f;
	}
	if (pointLight2_->position_.y > 4.0f || pointLight2_->position_.y < 0.0f) {
		pl2Velocity_.y *= -1.0f;
	}
	if (pointLight2_->position_.z > 4.0f || pointLight2_->position_.z < -4.0f) {
		pl2Velocity_.z *= -1.0f;
	}



	//点光源とスポットライトの座標にマークをつける
	Sphere sphere;
	Sphere sphere2;
	sphere.center = pointLight1_->position_;
	sphere.radius = 0.1f;
	MyMath::DrawSphere(sphere, { 1,1,0,1 }, line_.get(), 5);
	sphere.center = pointLight2_->position_;
	sphere.radius = 0.1f;
	MyMath::DrawSphere(sphere, { 1,1,0,1 }, line_.get(), 5);
	sphere.center = spotLight1_->position_;
	sphere.radius = 0.1f;
	sphere2.center = spotLight1_->position_ + (spotLight1_->direction_.Normalized() * 0.01f);
	sphere2.radius = 0.05f;
	MyMath::DrawSphere(sphere, { 0,1,0,1 }, line_.get(), 5);
	MyMath::DrawSphere(sphere2, { 1,0.25f,0,1 }, line_.get(), 5);
	sphere.center = spotLight2_->position_;
	sphere.radius = 0.1f;
	sphere2.center = spotLight2_->position_ + (spotLight2_->direction_.Normalized() * 0.01f);
	sphere2.radius = 0.05f;
	MyMath::DrawSphere(sphere, { 0,1,0,1 }, line_.get(), 5);
	MyMath::DrawSphere(sphere2, { 1,0.25f,0,1 }, line_.get(), 5);


#endif // _DEBUG
}

void DevelopScene::Draw()
{
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	sphere_->Draw(wtSphere_, *camera.get(), sceneLight_.get(), thSphere_);

	planeObj_->Draw(wtPlaneObj_, *camera.get(), sceneLight_.get());

	planeGltf_->Draw(wtPlaneGltf_, *camera.get(), sceneLight_.get());


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
