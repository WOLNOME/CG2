#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <numbers>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "Vector4.h"
#include "Matrix4x4.h"
#include "MyMath.h"


class LineDrawerManager
{
private://コンストラクタ等の隠蔽
	static LineDrawerManager* instance;

	LineDrawerManager() = default;//コンストラクタ隠蔽
	~LineDrawerManager() = default;//デストラクタ隠蔽
	LineDrawerManager(LineDrawerManager&) = delete;//コピーコンストラクタ封印
	LineDrawerManager& operator=(LineDrawerManager&) = delete;//コピー代入演算子封印
public:
	//シングルトンインスタンスの取得
	static LineDrawerManager* GetInstance();
public://構造体
	

public://メンバ関数
	//初期化
	void Initialize();
	//終了
	void Finalize();
	//更新
	void Update();
	//描画
	void Draw();

	//ライン描画リソースを受け取る
	uint32_t RegistrationLineResource(LineResource* resource);
private://非公開メンバ関数
	//ライングループ用リソース作成関数
	LineGroupResource MakeLineGroupResource();
	//SRVの設定
	void SettingSRV();

private://メンバ変数
	//表示するインスタンスの数
	static const uint32_t kNumMaxInstance_ = 128;
	//ラインリソースコンテナ
	std::unordered_map<uint32_t, LineResource*> lineResourceContainer_;
	//ライングループリソース(各インスタンシングの設定用リソース)
	LineGroupResource lineGroupResource_;
	//発券番号
	uint32_t registrationNumber = 0;



};

