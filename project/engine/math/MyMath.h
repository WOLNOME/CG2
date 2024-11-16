#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "LineDrawer.h"
#include <vector>

///------------------------------------///
///              列挙体
///------------------------------------///

//ライトの種類
enum LightKind {
	HalfLambert,
	Lambert,
	NoneLighting,
};

///------------------------------------///
///              構造体
///------------------------------------///

//トランスフォーム
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};
//球体
struct Sphere {
	Vector3 center;
	float radius;
};
//AABB
struct AABB {
	Vector3 min;
	Vector3 max;
};
//OBB
struct OBB {
	Vector3 center;				//!< 中心点
	Vector3 orientations[3];	//!< 座標軸。正規化・直交必須
	Vector3 size;				//!< 座標軸方向の長さの半分。中心から面までの距離
};
//直線
struct Line {
	Vector3 origin;//始点
	Vector3 diff;//終点への差分ベクトル
};
//半直線
struct Ray {
	Vector3 origin;//始点
	Vector3 diff;//終点への差分ベクトル
};
//線分
struct Segment {
	Vector3 origin;//始点
	Vector3 diff;//終点への差分ベクトル
};
//平面
struct Plane {
	Vector3 normal;//法線
	float distance;//距離
};
//三角形
struct Triangle {
	Vector3 vertices[3];
};
//ばね
struct Spring {
	Vector3 anchor;
	float naturalLength;
	float stiffness;
	float dampingCoefficient;
};
//ボール(物理)
struct Ball {
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	float mass;
	float radius;
	unsigned int color;
};
//2D矩形(物理)
struct Box {
	Vector2 pos;			// 位置
	Vector2 size;			// サイズ
	Vector2 velocity;		// 速度
	Vector2 accleration;	// 加速度
	float mass;				// 質量
	unsigned int color;		// 色
};
//振り子
struct Pendulum {
	Vector3 anchor;
	float length;
	float angle;
	float angularVelocity;
	float angularAcceleration;
};
//円錐振り子
struct ConicalPendulum {
	Vector3 anchor;
	float length;
	float halfApexAngle;
	float angle;
	float angularVelocity;
};
//トンネリング対策(カプセル)
struct Capsule {
	Segment segment;
	float radius;
};

class MyMath
{
public://静的メンバ関数
	///------------------------------------///
	///               Vector2
	///------------------------------------///

	//加算
	static Vector2 Add(const Vector2& v1, const Vector2& v2);
	//減算
	static Vector2 Subtract(const Vector2& v1, const Vector2& v2);
	//乗算
	static Vector2 Multiply(float s, const Vector2& v);

	///------------------------------------///
	///               Vector3
	///------------------------------------///

	//加算
	static Vector3 Add(const Vector3& v1, const Vector3& v2);
	//減算
	static Vector3 Subtract(const Vector3& v1, const Vector3& v2);
	//乗算
	static Vector3 Multiply(float s, const Vector3& v);
	//クロス積(外積)
	static Vector3 Cross(const Vector3& a, const Vector3& b);
	//正規化
	static Vector3 Normalize(const Vector3& v);
	//線形補完
	static Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
	//球面線形補完
	static Vector3 Slerp(const Vector3& vector1, const Vector3& vector2, float t);
	//座標変換
	static Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);
	//ローカル座標の座標変換
	static Vector3 TransformNormal(const Vector3& vector, const Matrix4x4& matrix);
	//正射影ベクトル
	static Vector3 Project(const Vector3& v1, const Vector3& v2);
	//最近接点
	static Vector3 ClosestPoint(const Vector3& point, const Segment& segment);
	//平面の法線から矩形を構成する4頂点をもとめる関数
	static Vector3 Perpendicular(const Vector3& vector);
	//AABBと球の中心の最近接点
	static Vector3 ClosestPoint(const AABB& aabb, const Sphere& sphere);
	//平面と直線の衝突点
	static Vector3 CollisionPoint(const Line& l, const Plane& p);
	//平面と半直線の衝突点
	static Vector3 CollisionPoint(const Ray& r, const Plane& p);
	//平面と線分の衝突点
	static Vector3 CollisionPoint(const Segment& s, const Plane& p);
	//反射ベクトルを求める関数
	static Vector3 Reflect(const Vector3& input, const Vector3& normal);



	///------------------------------------///
	///               Matrix4x4
	///------------------------------------///

	//加算
	static Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);
	//減算
	static Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);
	//乗算
	static Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
	//逆行列
	static Matrix4x4 Inverse(const Matrix4x4& m);
	//転置行列
	static Matrix4x4 Transpose(const Matrix4x4& m);
	//単位行列
	static Matrix4x4 MakeIdentity4x4();
	//平行移動行列
	static Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
	//スケール行列
	static Matrix4x4 MakeScaleMatrix(const Vector3& scale);
	//X軸回転行列
	static Matrix4x4 MakeRotateXMatrix(float radian);
	//Y軸回転行列
	static Matrix4x4 MakeRotateYMatrix(float radian);
	//Z軸回転行列
	static Matrix4x4 MakeRotateZMatrix(float radian);
	//アフィン変換行列
	static Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
	//射影変換行列
	static Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
	//平行投影行列
	static Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
	//ビューポート行列
	static Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	///------------------------------------///
	///               float
	///------------------------------------///

	//余接
	static float Cot(float rad);
	//距離
	static float Length(const Vector3& v);
	//ベクトルの内積
	static float Dot(const Vector3& v1, const Vector3& v2);
	//スカラーの線形補完
	static float Lerp(float s1, float s2, float t);

	///------------------------------------///
	///             補助関数
	///------------------------------------///

	//ベクトルを指定した軸に射影し、その最小値と最大値を計算する関数
	static std::pair<float, float> ProjectOntoAxis(const Vector3* vertices, int count, const Vector3& axis);


	///------------------------------------///
	///      イージングチートシート
	///------------------------------------///

	static float EaseInSine(float ratio);

	static float EaseOutSine(float ratio);

	static float EaseInOutSine(float ratio);

	static float EaseInCubic(float ratio);

	static float EaseOutCubic(float ratio);

	static float EaseInOutCubic(float ratio);

	static float EaseInQuint(float ratio);

	static float EaseOutQuint(float ratio);

	static float EaseInOutQuint(float ratio);

	static float EaseInCirc(float ratio);

	static float EaseOutCirc(float ratio);

	static float EaseInOutCirc(float ratio);

	static float EaseInElastic(float ratio);

	static float EaseOutElastic(float ratio);

	static float EaseInOutElastic(float ratio);

	static float EaseInQuad(float ratio);

	static float EaseOutQuad(float ratio);

	static float EaseInOutQuad(float ratio);

	static float EaseInQuart(float ratio);

	static float EaseOutQuart(float ratio);

	static float EaseInOutQuart(float ratio);

	static float EaseInExpo(float ratio);

	static float EaseOutExpo(float ratio);

	static float EaseInOutExpo(float ratio);

	static float EaseInBack(float ratio);

	static float EaseOutBack(float ratio);

	static float EaseInOutBack(float ratio);

	static float EaseInBounce(float ratio);

	static float EaseOutBounce(float ratio);

	static float EaseInOutBounce(float ratio);

	///------------------------------------///
	///               当たり判定
	///------------------------------------///

	//球同士の当たり判定
	static bool IsCollision(const Sphere& s1, const Sphere& s2);
	//平面と球の当たり判定
	static bool IsCollision(const Plane& plane, const Sphere& sphere);
	//直線と平面の当たり判定
	static bool IsCollision(const Line& line, const Plane& plane);
	//半直線と平面の当たり判定
	static bool IsCollision(const Ray& ray, const Plane& plane);
	//線分と平面の当たり判定
	static bool IsCollision(const Segment& segment, const Plane& plane);
	//線分と三角形の当たり判定
	static bool IsCollision(const Segment& segment, const Triangle& triangle);
	//AABB同士の当たり判定
	static bool IsCollision(const AABB& a, const AABB& b);
	//AABBと点の当たり判定
	static bool IsCollision(const AABB& aabb, const Vector3& point);
	//AABBと球の当たり判定
	static bool IsCollision(const AABB& aabb, const Sphere& sphere);
	//AABBと直線の当たり判定
	static bool IsCollision(const AABB& aabb, const Line& line);
	//AABBと半直線の当たり判定
	static bool IsCollision(const AABB& aabb, const Ray& ray);
	//AABBと線分の当たり判定
	static bool IsCollision(const AABB& aabb, const Segment& segment);
	//OBB同士の当たり判定
	static bool IsCollision(const OBB& obb1, const OBB& obb2);
	//OBBと球の当たり判定
	static bool IsCollision(const OBB& obb, const Sphere& sphere);
	//OBBと直線の当たり判定
	static bool IsCollision(const OBB& obb, const Line& line);
	//OBBと半直線の当たり判定
	static bool IsCollision(const OBB& obb, const Ray& ray);
	//OBBと線分の当たり判定
	static bool IsCollision(const OBB& obb, const Segment& segment);

	///------------------------------------///
	///              図形の線描画
	///------------------------------------///
	
	static void DrawSphere(const Sphere& sphere, Vector4 color, LineDrawer* lineDrawer);

};

///------------------------------------///
///       演算子のオーバーロード
///------------------------------------///

//二項演算子
Vector2 operator+(const Vector2& v1, const Vector2& v2);
Vector2 operator-(const Vector2& v1, const Vector2& v2);
Vector2 operator*(float s, const Vector2& v);
Vector2 operator*(const Vector2& v, float s);
Vector2 operator/(const Vector2& v, float s);
Vector3 operator+(const Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v1, const Vector3& v2);
Vector3 operator*(float s, const Vector3& v);
Vector3 operator*(const Vector3& v, float s);
Vector3 operator/(const Vector3& v, float s);
Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);
//単項演算子
Vector3 operator-(const Vector3& v);
Vector3 operator+(const Vector3& v);



