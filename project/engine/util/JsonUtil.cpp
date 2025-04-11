#include "JsonUtil.h"
#include <fstream>
#include <filesystem>
#include <cassert>

namespace fs = std::filesystem;

std::optional<json> JsonUtil::GetJsonData(const std::string& filePath) {
	std::string fullPath = filePath + ".json";

	//ファイルが存在するかチェック
	if (!fs::exists(fullPath)) {
		return std::nullopt;
	}

	//ファイルを開く
	std::ifstream file(fullPath);
	if (!file.is_open()) {
		return std::nullopt;
	}

	//JSONデータを読み込む
	json jsonData;
	try {
		file >> jsonData;
	}
	catch (json::parse_error&) {
		return std::nullopt;
	}

	return jsonData;
}

bool JsonUtil::EditJson(const std::string& filePath, const json& jsonData) {
	//ファイルが存在しているか確認
	std::string fullPath = filePath + ".json";
	if (!fs::exists(fullPath)) {
		//ファイルが存在していなかったらfalseを返す
		return false;
	}

	//jsonDataをファイルに上書き
	std::ofstream file(fullPath);
	//4スペースでシリアライズ(整列)
	file << jsonData.dump(4);
	file.close();

	return true;
}

bool JsonUtil::CreateJson(const std::string& name, const std::string& targetDir, const json& jsonData) {
	//すでにファイルが存在しているか確認
	std::string fullPath = targetDir + name + ".json";
	if (fs::exists(fullPath)) {
		//すでにファイルが存在していたらfalseを返す
		return false;
	}

	//targetDirに新しくJSONファイルを作成
	std::ofstream file(fullPath);
	if (!file) {
		//ファイルの作成に失敗したらfalseを返す
		return false;
	}
	//4スペースでシリアライズ(整列)
	file << jsonData.dump(4);
	file.close();

	return true;


}

bool JsonUtil::CheckJson(const std::string& name, const std::string& targetDir) {
	std::string fullName = name + ".json";
	std::string targetPath = targetDir;
	if (!targetPath.empty() && targetPath.back() == '/') {
		targetPath.erase(targetPath.size() - 1);
	}
	//フォルダ内の全ファイルをループ
	for (const auto& entry : fs::directory_iterator(targetPath)) {
		if (entry.path().filename().string() == fullName) {
			//一致するものを見つけたのでtrue
			return true;
		}
	}
	//無かったのでfalse
	return false;
}
