#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <optional>
#include <map>
#include <string>
#include "WorldTransform.h"
#include "BaseCamera.h"
#include "MyMath.h"
#include "ModelFormat.h"
#include "LineDrawer.h"
//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
private://アニメーション関連構造体
	//キーフレーム
	template <typename tValue>
	struct Keyframe
	{
		float time;
		tValue value;
	};
	using keyframeVector3 = Keyframe<Vector3>;
	using keyframeQuaternion = Keyframe<Quaternion>;
	//ノードアニメーション
	template <typename tValue>
	struct AnimationCurve
	{
		std::vector<Keyframe<tValue>> keyframes;
	};

	struct NodeAnimation {
		AnimationCurve<Vector3> translate;
		AnimationCurve<Quaternion> rotate;
		AnimationCurve<Vector3> scale;
	};
	//アニメーション
	struct Animation
	{
		float duration;//アニメーション全体の尺(秒)
		//NodeAnimationの集合、Node名で開けるようにしておく
		std::map<std::string, NodeAnimation> nodeAnimations;
	};
	//joint
	struct Joint {
		TransformQuaternion transform;
		Matrix4x4 localMatrix;
		Matrix4x4 skeletonSpaceMatrix;
		std::string name;
		std::vector<int32_t> children;
		int32_t index;
		std::optional<int32_t> parent;
	};
	//skeleton
	struct Skeleton {
		int32_t root;
		std::map<std::string, int32_t> jointMap;
		std::vector<Joint> joints;
	};

private://メッシュ関連構造体
	//頂点データ
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	//マテリアル
	struct Material {
		Vector4 color;
		Matrix4x4 uvTransform;
		float isTexture;
		float shininess;
	};
	//ノード
	struct Node {
		TransformQuaternion transform;
		Matrix4x4 localMatrix;
		std::string name;
		std::vector<Node> children;
	};
	//マテリアルデータ
	struct MaterialData {
		std::string materialName;
		std::string textureFilePath;
		Vector4 colorData;
		uint32_t textureHandle;
	};
	//モデルデータ
	struct ModelData {
		std::vector<VertexData> vertices;
		std::vector<uint32_t> indices;
		MaterialData material;
		Node rootNode;
	};
	//モデルリソース作成用データ型
	struct ModelResource {
		std::vector<ModelData> modelData;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> vertexResource;
		std::vector<D3D12_VERTEX_BUFFER_VIEW> vertexBufferView;
		std::vector<VertexData*> vertexData;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> indexResource;
		std::vector<D3D12_INDEX_BUFFER_VIEW> indexBufferView;
		std::vector<uint32_t*> indexData;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> materialResource;
		std::vector<Material*> materialData;
		std::vector<TransformEuler> uvTransform;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResorce;
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textureSrvHandleCPU;
		std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> textureSrvHandleGPU;
	};
	

public:
	void Initialize(const std::string& filename, ModelFormat format = OBJ, std::string directorypath = "Resources/models/");
	void Update();
	/// <summary>
	/// モデル描画
	/// </summary>
	/// <param name="materialRootParameterIndex">マテリアル設定用ルートパラメータの番号</param>
	/// <param name="textureRootParameterIndex">テクスチャ設定用ルートパラメータの番号</param>
	/// <param name="instancingNum">インスタンス数</param>
	void Draw(uint32_t materialRootParameterIndex, uint32_t textureRootParameterIndex, uint32_t instancingNum = 1);
	/// <summary>
	/// デバッグ用線描画
	/// </summary>
	/// <param name="worldTransform">ワールドトランスドーム</param>
	/// <param name="camera">カメラ</param>
	void DrawLine(const WorldTransform& worldTransform, const BaseCamera& camera);

public://ゲッター
	const ModelResource& GetModelResource() { return modelResource_; }
	
public://セッター

private:
	//モデルファイルの読み取り
	std::vector<ModelData> LoadModelFile();
	//アニメーションの読み取り
	Animation LoadAnimationFile();
	//assimpのノード→構造体ノード変換関数
	Node ReadNode(aiNode* node);
	//モデルリソース作成関数
	ModelResource MakeModelResource();
	//テクスチャ読み込み
	void SettingTexture();

	//任意の時刻に対する値を取得する関数
	Vector3 CalculateValue(const std::vector<Keyframe<Vector3>>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<Keyframe<Quaternion>>& keyframes, float time);
	//NodeからJointを作り出す
	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints);
	//NodeからSkeletonを作り出す関数
	Skeleton CreateSkeleton(const Node& rootNode);
	//joint(骨)の更新
	void UpdateJoints(Skeleton& skeleton);
	//アニメーションを適用する関数
	void ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime);

private:
	//モデル用リソース
	ModelResource modelResource_;
	//モデル数
	size_t modelNum_;

	//ディレクトリパス
	std::string directoryPath_;
	//モデルデータファイル名
	std::string fileName_;
	//形式名
	ModelFormat mf_;
	std::string format_;

	//アニメーション用変数
	Animation animation_;
	float animationTime_ = 0.0f;
	bool isAnimation_ = false;

	//スケルトン
	Skeleton skeleton_;
	bool isSkeleton_ = false;

	//骨と関節のデバッグ表示
	std::vector<std::unique_ptr<LineDrawer>> lines_;
	float jointRadius_ = 0.01f;
};

