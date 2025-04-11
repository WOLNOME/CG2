#pragma once
#include "json.hpp"
#include <string>
#include <optional>

using json = nlohmann::json;

class JsonUtil {
public:
	/// <summary>
	/// Jsonデータを出力する
	/// </summary>
	/// <param name="filePath">ファイルパス(.jsonは省略)</param>
	/// <returns>jsonデータ</returns>
	static std::optional<json> GetJsonData(const std::string& filePath);
	/// <summary>
	/// Jsonファイルを編集する
	/// </summary>
	/// <param name="filePath">ファイルパス(.jsonは省略)</param>
	/// <param name="jsonData">jsonデータ</param>
	static bool EditJson(const std::string& filePath, const json& jsonData);
	/// <summary>
	/// Jsonファイルを新しく作成する
	/// </summary>
	/// <param name="name">ファイル名(.jsonは省略)</param>
	/// <param name="targetDir">作成するフォルダのパス(/まで入れる)</param>
	/// <param name="jsonData">jsonデータ</param>
	static bool CreateJson(const std::string& name, const std::string& targetDir, const json& jsonData);
	/// <summary>
	/// Jsonファイルの有無を検索
	/// </summary>
	/// <param name="name">検索したいJsonファイル名(.jsonは省略)</param>
	/// <param name="targetDir">検索したいフォルダのパス(/まで入れる)</param>
	/// <returns></returns>
	static bool CheckJson(const std::string& name,const std::string& targetDir);
};

