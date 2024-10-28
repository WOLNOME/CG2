#include "MyGame.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//生成と初期化
	MyGame* myGame = nullptr;
	myGame = new MyGame();
	myGame->Initialize();
	//ウィンドウの×ボタンが押されるまでループ
	while (true) {

		//更新
		myGame->Update();
		//ループ脱出
		if (myGame->GetOver()) {
			break;
		}
		//描画
		myGame->Draw();

	}
	//解放
	myGame->Finalize();
	delete myGame;

	return 0;
}
