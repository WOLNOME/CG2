#include "SceneFactory.h"
#include "DevelopScene.h"
#include "EvaluationTaskScene.h"
#include "TitleScene.h"
#include "ClearScene.h"

// パーティクルクリエイター
#include "ParticleCreatorScene.h"

BaseScene* SceneFactory::CreateScene(const std::string& sceneName)
{
	//次のシーンを生成
	BaseScene* newScene = nullptr;

	if (sceneName == "DEVELOP") {
		newScene = new DevelopScene();
	}
	else if (sceneName == "EVALUATIONTASK") {
		newScene = new EvaluationTaskScene();
	}
	
	//パーティクルクリエイター
	else if (sceneName == "PARTICLECREATOR") {
		newScene = new ParticleCreatorScene();
	}

	return newScene;
}
