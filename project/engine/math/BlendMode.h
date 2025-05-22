#pragma once

enum class BlendMode {
	None,				//ブレンドなし
	Normal,				//通常
	Add,				//加算
	Subtract,			//減算
	Multiply,			//乗算
	Screen,				//スクリーン
	Execlution,			//除外

	kMaxBlendModeNum,	//ブレンドモードの最大数
};
