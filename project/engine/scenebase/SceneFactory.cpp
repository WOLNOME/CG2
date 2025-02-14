#include "SceneFactory.h"
#include "DevelopScene.h"
#include "TitleScene.h"
#include "ClearScene.h"

BaseScene* SceneFactory::CreateScene(const std::string& sceneName)
{
	//次のシーンを生成
	BaseScene* newScene = nullptr;

	if (sceneName == "DEVELOP") {
		newScene = new DevelopScene();
	}
	else if (sceneName == "TITLE") {
		newScene = new TitleScene();
	}
	else if (sceneName == "CLEAR") {
		newScene = new ClearScene();
	}

	return newScene;
}
