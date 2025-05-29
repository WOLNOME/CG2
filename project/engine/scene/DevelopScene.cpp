#include "DevelopScene.h"
#include "SceneManager.h"
#include <numbers>


void DevelopScene::Initialize() {
	//シーン共通の初期化
	BaseScene::Initialize();

	input_ = Input::GetInstance();

	//カメラの生成と初期化
	camera = std::make_unique<DevelopCamera>();
	camera->Initialize();
	camera->SetRotate({ cameraRotate });
	camera->SetTranslate(cameraTranslate);
	camera->SetFarClip(500.0f);

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
	sprite_->Initialize();
	sprite_->AdjustTextureSize(textureHandleSprite_);
	sprite_->SetAnchorPoint({ 0.5f,0.5f });
	sprite_->SetFlipX(true);

	sprite2_ = std::make_unique<Sprite>();
	textureHandleSprite2_ = TextureManager::GetInstance()->LoadTexture("monsterBall.png");
	sprite2_->Initialize();
	sprite_->AdjustTextureSize(textureHandleSprite2_);
	sprite2Position = { 100.0f,100.0f };
	sprite2_->SetPosition(sprite2Position);
	sprite2_->SetSize({ 300.0f,300.0f });

	//スカイボックスの生成と初期化
	textureHandleSkyBox_ = TextureManager::GetInstance()->LoadTexture("rostock_laage_airport_4k.dds");
	skyBox_ = std::make_unique<Object3d>();
	skyBox_->Initialize(ShapeTag{}, Shape::ShapeKind::kSkyBox);
	skyBox_->worldTransform.scale = { 300.0f,300.0f,300.0f };

	//3Dオブジェクトの生成と初期化
	teapot_ = std::make_unique<Object3d>();
	teapot_->Initialize(ModelTag{}, "teapot");
	int32_t elthTeapot = TextureManager::GetInstance()->LoadTexture("rostock_laage_airport_4k.dds");
	teapot_->SetEnvironmentLightTextureHandle(elthTeapot);
	teapot_->SetSceneLight(sceneLight_.get());

	terrain_ = std::make_unique<Object3d>();
	terrain_->Initialize(ModelTag{}, "terrain");
	terrain_->worldTransform.translate = { 0.0f,-1.2f,0.0f };
	terrain_->SetSceneLight(sceneLight_.get());

	animatedCube_ = std::make_unique<Object3d>();
	animatedCube_->Initialize(AnimationModelTag{}, "AnimatedCube", GLTF);
	animatedCube_->worldTransform.translate = { 0.0f,3.0f,0.0f };
	animatedCube_->SetSceneLight(sceneLight_.get());

	sneakWalk_ = std::make_unique<Object3d>();
	sneakWalk_->Initialize(AnimationModelTag{}, "sneakWalk", GLTF);
	sneakWalk_->worldTransform.translate = { 3.0f,3.0f,0.0f };
	sneakWalk_->SetSceneLight(sceneLight_.get());
	sneakWalk_->SetNewAnimation("sneakWalk", "sneakWalk");
	sneakWalk_->SetCurrentAnimation("sneakWalk");

	composite_ = std::make_unique<Object3d>();
	composite_->Initialize(AnimationModelTag{}, "walk", GLTF);
	composite_->worldTransform.translate = { 0.0f,1.0f,-3.0f };
	composite_->worldTransform.rotate = { 0.0f,3.14f,0.0f };
	composite_->SetSceneLight(sceneLight_.get());
	composite_->SetNewAnimation("walk", "walk");
	composite_->SetNewAnimation("sneakWalk", "sneakWalk");
	composite_->SetCurrentAnimation("walk");

	walk_ = std::make_unique<Object3d>();
	walk_->Initialize(AnimationModelTag{}, "walk", GLTF);
	int32_t elthWalk = TextureManager::GetInstance()->LoadTexture("rostock_laage_airport_4k.dds");
	walk_->SetEnvironmentLightTextureHandle(elthWalk);
	walk_->worldTransform.translate = { 4.0f,3.0f,0.0f };
	walk_->SetSceneLight(sceneLight_.get());
	walk_->SetNewAnimation("walk", "walk");
	walk_->SetCurrentAnimation("walk");

	simpleSkin_ = std::make_unique<Object3d>();
	simpleSkin_->Initialize(AnimationModelTag{}, "simpleSkin", GLTF);
	simpleSkin_->worldTransform.translate = { 5.0f,3.0f,0.0f };
	simpleSkin_->SetSceneLight(sceneLight_.get());

	ParticleManager::GetInstance()->SetCamera(camera.get());
	//particle_ = std::make_unique<Particle>();
	//particle_->Initialize("develop", "basic");

	line_ = std::make_unique<LineDrawer>();
	line_->Initialize();

	audio_ = std::make_unique<Audio>();
	audio_->Initialize("demo2.wav");

	//テキストテクスチャの作成
	{
		TextParam param;
		param.text = L"フォント確認 0123 abcDEF";
		param.font = Font::UDDegitalNP_B;
		param.fontStyle = FontStyle::Normal;
		param.size = 32.0f;
		param.color = { 1,1,1,1 };
		textHandle_ = TextTextureManager::GetInstance()->LoadTextTexture(param);
		text_ = std::make_unique<Sprite>();
		text_->Initialize();
		text_->SetPosition({ 640,360 });
		text_->SetAnchorPoint({ 0.5f,0.5f });
		EdgeParam edgeParam;
		edgeParam.width = 2;
		edgeParam.isEdgeDisplay = 1;
		edgeParam.slideRate = { 0.0f,0.0f };
		edgeParam.color = { 1,1,0,1 };
		TextTextureManager::GetInstance()->EditEdgeParam(textHandle_, edgeParam);
	}

	//テクスチャ
	{
		TextParam param;
		param.text = L"大量のテキストを投下。";
		param.font = Font::UDDegitalNP_B;
		param.fontStyle = FontStyle::Normal;
		param.size = 90.0f;
		param.color = { 1,1,1,1 };
		handle2_ = TextTextureManager::GetInstance()->LoadTextTexture(param);
		text2_ = std::make_unique<Sprite>();
		text2_->Initialize();
		text2_->AdjustTextureSize(handle2_);
		text2_->SetPosition({ text2_->GetPosition().x,text2_->GetPosition().y + 100.0f });
		EdgeParam edgeParam;
		edgeParam.width = 5;
		edgeParam.isEdgeDisplay = 1;
		edgeParam.slideRate = { 0.0f,0.0f };
		edgeParam.color = { 0,0,1,1 };
		TextTextureManager::GetInstance()->EditEdgeParam(handle2_, edgeParam);
	}
	//テクスチャ複数作成
	for (int i = 0; i < 6; i++) {
		TextParam param;
		param.text = L"テクスチャ複数作成実験";
		param.font = Font::UDDegitalNP_B;
		param.fontStyle = FontStyle::Normal;
		param.size = 90.0f;
		param.color = { 1,1,1,1 };
		handles_[i] = TextTextureManager::GetInstance()->LoadTextTexture(param);
		EdgeParam edgeParam;
		edgeParam.width = 5;
		edgeParam.isEdgeDisplay = 0;
		edgeParam.slideRate = { 0.0f,0.0f };
		edgeParam.color = { 0,0,1,1 };
		TextTextureManager::GetInstance()->EditEdgeParam(handles_[i], edgeParam);
	}
}

void DevelopScene::Finalize() {
}

void DevelopScene::Update() {
	//シーン共通の更新
	BaseScene::Update();

	//TABでシーン再読み込み
	if (input_->TriggerKey(DIK_TAB)) {
		sceneManager_->SetNextScene("DEVELOP");
	}

	//カメラの更新
	camera->Update();

	//スカイボックスの更新
	skyBox_->Update();

	//ティーポットの回転
	teapot_->worldTransform.rotate.y += 0.03f;
	//オブジェクトの更新
	teapot_->Update();
	terrain_->Update();
	animatedCube_->Update();
	walk_->Update();
	sneakWalk_->Update();
	composite_->Update();
	simpleSkin_->Update();

	//スプライトの更新
	sprite_->SetRotation(sprite_->GetRotation() + 0.03f);

	time_ += kDeltaTime;
	TextTextureManager::GetInstance()->EditTextString(textHandle_, L"フォント確認 0123 abcDEF\n現在時刻 : {:.1f}", time_);

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

	ImGui::Begin("複合アニメーション");
	//選択肢
	const char* items[] = { "walk","sneakWalk" };
	static int currentItem = 0;
	if (ImGui::Combo("アニメーションのアイテム", &currentItem, items, IM_ARRAYSIZE(items))) {
		composite_->SetCurrentAnimation(items[currentItem]);
	}
	ImGui::End();

	//テキスト用ImGui
	TextTextureManager::GetInstance()->DebugWithImGui(textHandle_);
	//カメラ用ImGui
	camera->DebugWithImGui();
	//ポストエフェクト用ImGui
	PostEffectManager::GetInstance()->DebugWithImGui();

#endif // _DEBUG
}

void DevelopScene::Draw() {
	//3Dモデルの共通描画設定
	Object3dCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓モデル描画開始↓↓↓↓
	///------------------------------///

	//スカイボックス描画
	skyBox_->Draw(camera.get(), textureHandleSkyBox_);

	/*terrain_->Draw(camera.get());
	teapot_->Draw(camera.get());

	animatedCube_->Draw(camera.get());

	walk_->Draw(camera.get());

	sneakWalk_->Draw(camera.get());

	composite_->Draw(camera.get());

	simpleSkin_->Draw(camera.get());*/

	///------------------------------///
	///↑↑↑↑モデル描画終了↑↑↑↑
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

	///------------------------------///
	///↑↑↑↑線描画終了↑↑↑↑
	///------------------------------///

	//スプライトの共通描画設定
	SpriteCommon::GetInstance()->SettingCommonDrawing();

	///------------------------------///
	///↓↓↓↓スプライト描画開始↓↓↓↓
	///------------------------------///

	////スプライト描画
	//sprite_->Draw(textureHandleSprite_);
	//sprite2_->Draw();

	//テキスト
	text_->AdjustTextureSize(textHandle_);
	text_->Draw(textHandle_);
	text2_->AdjustTextureSize(handle2_);
	text2_->Draw(handle2_);


	///------------------------------///
	///↑↑↑↑スプライト描画終了↑↑↑↑
	///------------------------------///



}
