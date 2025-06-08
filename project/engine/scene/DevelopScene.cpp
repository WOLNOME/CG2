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

	//レベルオブジェクトの生成・初期化
	levelObject_ = std::make_unique<LevelObject>();
	levelObject_->Initialize("Resources/levelOutput");
	levelObject_->SetCamera(camera.get());

	//パーティクルの生成・初期化
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
	//レベルオブジェクトの更新
	levelObject_->Update();

	//スプライトの更新
	sprite_->SetRotation(sprite_->GetRotation() + 0.03f);

	time_ += kDeltaTime;
	TextTextureManager::GetInstance()->EditTextString(textHandle_, L"フォント確認 0123 abcDEF\n現在時刻 : {:.1f}", time_);

#ifdef _DEBUG
	//オーディオのデバッグ用ImGui
	audio_->DebugWithImGui(L"アラーム");

	//平行光源のデバッグ用ImGui
	dirLight->DebugWithImGui(L"1");

	//点光源のデバッグ用ImGui
	pointLight->DebugWithImGui(L"1");
	pointLight2->DebugWithImGui(L"2");

	//スポットライトのデバッグ用ImGui
	spotLight->DebugWithImGui(L"1");

	ImGui::Begin("複合アニメーション");
	//選択肢
	const char* items[] = { "walk","sneakWalk" };
	static int currentItem = 0;
	if (ImGui::Combo("アニメーションのアイテム", &currentItem, items, IM_ARRAYSIZE(items))) {
		composite_->SetCurrentAnimation(items[currentItem]);
	}
	ImGui::End();

	//レベルオブジェクト用ImGui
	levelObject_->DebugWithImGui();
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

	terrain_->Draw(camera.get());
	teapot_->Draw(camera.get());

	animatedCube_->Draw(camera.get());

	walk_->Draw(camera.get());

	sneakWalk_->Draw(camera.get());

	composite_->Draw(camera.get());

	simpleSkin_->Draw(camera.get());

	levelObject_->Draw();

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

	///------------------------------///
	///↑↑↑↑スプライト描画終了↑↑↑↑
	///------------------------------///



}
