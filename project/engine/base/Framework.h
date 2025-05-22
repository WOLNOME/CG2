#pragma once
#include "D3DResourceLeakChecker.h"
#include <memory>

class Framework
{
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~Framework() = default;
	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize();
	/// <summary>
	/// 終了時
	/// </summary>
	virtual void Finalize();
	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update();
	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw() = 0;
	/// <summary>
	/// 終了判定
	/// </summary>
	/// <returns></returns>
	virtual bool GetOver() { return isOver; }
public:
	//実行
	void Run();
protected://基盤インスタンス
	D3DResourceLeakChecker leakChecker;
	bool isOver = false;
	
};

