#include <fstream>
#include <xaudio2.h>
#include "DirectXCommon.h"
#include "WinApp.h"
#include "Input.h"
#include "D3DResourceLeakChecker.h"
#include "Logger.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "Model.h"
#include "TextureManager.h"
#include "Function.h"
#include "ParticleCommon.h"
#include "Particle.h"

#pragma comment(lib,"xaudio2.lib")

//定数
const int kTriangleVertexNum = 3;
const int kTriangleNum = 2;

const uint32_t kSubdivision = 20;

//チャンクヘッダ
struct ChunkHeader
{
	char id[4];
	int32_t size;
};

//RIFFヘッダチャンク
struct RiffHeader
{
	ChunkHeader chunk;
	char type[4];
};

//FMTチャンク
struct FormatChunk
{
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

//音声データ
struct SoundData
{
	//波形フォーマット
	WAVEFORMATEX wfex;
	//バッファの先頭アドレス
	BYTE* pBuffer;
	//バッファのサイズ
	unsigned int bufferSize;
};

//音声データの読み込み
SoundData SoundLoadWave(const char* filename)
{
	//1.ファイルオープン
	std::ifstream file;
	file.open(filename, std::ios_base::binary);
	assert(file.is_open());
	//2.「.wav」データ読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}
	FormatChunk format = {};
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);
	//3.ファイルクローズ
	file.close();
	//4.読み込んだ音声データをreturn
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

//音声データの解放
void SoundUnload(SoundData* soundData)
{
	//バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

//サウンドの再生
void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData)
{
	HRESULT result;
	//波形フォーマットからSourceVoiceを生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));
	//再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	//波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#pragma region 基盤システムの初期化
	//解放処理確認用
	D3DResourceLeakChecker leakChecker;

	//ウィンドウ
	WinApp* winApp = nullptr;
	//WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

	//DorectX12
	DirectXCommon* dxCommon = nullptr;
	//DirectXの初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//テクスチャマネージャ
	TextureManager::GetInstance()->Initialize(dxCommon);

	//モデルマネージャー
	ModelManager::GetInstance()->Initialize(dxCommon);

	//インプット
	Input* input = nullptr;
	input = new Input();
	input->Initialize(winApp);

	//スプライト共通部
	SpriteCommon* spriteCommon = nullptr;
	spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(dxCommon);

	//オブジェクト3D共通部
	Object3dCommon* object3dCommon = nullptr;
	object3dCommon = new Object3dCommon();
	object3dCommon->Initialize(dxCommon);

	//パーティクル共通部
	ParticleCommon* particleCommon = nullptr;
	particleCommon = new ParticleCommon();
	particleCommon->Initialize(dxCommon);

#pragma endregion 基盤システムの初期化

	//Transform変数を作る
	Transform transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};


	//カメラの位置、角度を作る
	Transform cameraTransform{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-10.0f}
	};

	//////////////////Xaudio2の設定/////////////////////////////////////
	//必要な変数の宣言
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;
	///////////////////////////////////////////////////////////////////


	//初期化
#pragma region 最初のシーン初期化
	Sprite* sprite = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	sprite->Initialize(spriteCommon, "Resources/uvChecker.png");
	sprite->SetAnchorPoint({ 0.5f,0.5f });
	sprite->SetFlipX(true);

	Sprite* sprite2 = new Sprite();
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");
	sprite2->Initialize(spriteCommon, "Resources/monsterBall.png");
	sprite2->SetPosition({ 0.0f,360.0f });

	ModelManager::GetInstance()->LoadModel("plane.obj");
	Object3d* object3d = new Object3d();
	object3d->SetModel("plane.obj");
	object3d->Initialize(object3dCommon);

	ModelManager::GetInstance()->LoadModel("axis.obj");
	Object3d* object3d2 = new Object3d();
	object3d2->SetModel("axis.obj");
	object3d2->Initialize(object3dCommon);

	Particle* particle = new Particle();
	particle->Initialize(particleCommon, 10);

#pragma endregion 最初のシーンの終了

	//ライト
	bool isLightingSphere = true;
	bool isHalfLambert = true;
	//XAudio2エンジンのインスタンス作成
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));
	//マスターボイスを作成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(hr));
	//音声読み込み
	SoundData soundData1 = SoundLoadWave("Resources/Alarm01.wav");
	//再生フラグ
	bool isPlayAudio = false;


	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		//メッセージ処理
		if (winApp->ProcessMessage()) {
			break;
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		input->Update();


		//ゲームの処理

		//////////////////////
		///更新処理
		//////////////////////

		//モデルの更新
		object3d->Update();
		object3d2->Update();

		particle->Update();

		//スプライトの更新
		sprite->Update();
		sprite->SetRotation(sprite->GetRotation() + 0.03f);
		sprite2->Update();


		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える

		ImGui::Begin("Settings");


		ImGui::End();

		/////レンダリングパイプライン/////

		//ImGuiの内部コマンドを生成する
		ImGui::Render();

		///==============================///
		///          描画処理
		///==============================///

		//描画前処理
		dxCommon->PreDraw();

		//3Dモデルの共通描画設定
		object3dCommon->SettingCommonDrawing();

		///------------------------------///
		///          モデル描画
		///------------------------------///

		object3d->Draw();
		object3d2->Draw();

		

		//パーティクル共通描画設定
		particleCommon->SettingCommonDrawing();

		///------------------------------///
		///          パーティクル描画
		///------------------------------///

		particle->Draw();


		//スプライトの共通描画設定
		spriteCommon->SettingCommonDrawing();

		///------------------------------///
		///          スプライト描画
		///------------------------------///

		//スプライト描画
		sprite->Draw();
		//sprite2->Draw();

		//ImGuiの描画
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

		//描画後処理
		dxCommon->PostDraw();
	}


	//ImGuiの終了処理。
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	/////解放処理/////

	//xAudio2
	xAudio2.Reset();
	//音声データ
	SoundUnload(&soundData1);

	delete particle;
	delete object3d;
	delete sprite2;
	delete sprite;
	delete particleCommon;
	delete object3dCommon;
	delete spriteCommon;
	delete input;
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	delete dxCommon;
	//WindowsAPIの終了処理
	winApp->Finalize();
	delete winApp;

	return 0;
}
