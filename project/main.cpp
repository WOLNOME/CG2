#include "Framework.h"
#include "MyGame.h"
#include "Windows.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	Framework* game = new MyGame();

	game->Run();

	delete game;

	return 0;
}
