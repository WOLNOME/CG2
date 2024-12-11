#include "MyMath.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <numbers>
#include "LineDrawer.h"

Vector2 MyMath::Add(const Vector2& v1, const Vector2& v2)
{
	Vector2 c;
	c = {
		v1.x + v2.x,
		v1.y + v2.y
	};
	return c;
}

Vector2 MyMath::Subtract(const Vector2& v1, const Vector2& v2)
{
	Vector2 c;
	c = {
		v1.x - v2.x,
		v1.y - v2.y
	};
	return c;
}

Vector2 MyMath::Multiply(float s, const Vector2& v)
{
	return Vector2(s * v.x, s * v.y);
}

Vector3 MyMath::Add(const Vector3& v1, const Vector3& v2)
{
	Vector3 c;
	c = {
		v1.x + v2.x,
		v1.y + v2.y,
		v1.z + v2.z
	};
	return c;
}

Vector3 MyMath::Subtract(const Vector3& v1, const Vector3& v2)
{
	Vector3 c;
	c = {
		v1.x - v2.x,
		v1.y - v2.y,
		v1.z - v2.z
	};
	return c;
}

Vector3 MyMath::Multiply(float s, const Vector3& v)
{
	return Vector3(s * v.x, s * v.y, s * v.z);
}

Vector3 MyMath::Cross(const Vector3& a, const Vector3& b)
{
	Vector3 c;

	c.x = (a.y * b.z) - (a.z * b.y);
	c.y = (a.z * b.x) - (a.x * b.z);
	c.z = (a.x * b.y) - (a.y * b.x);

	return c;
}

Vector3 MyMath::Normalize(const Vector3& v)
{
	Vector3 c;
	//長さを求める
	float length = Length(v);
	//length=0で無ければ正規化
	if (length != 0) {
		c.x = v.x / length;
		c.y = v.y / length;
		c.z = v.z / length;
	}
	else {
		assert("正規化できません");
	}
	return c;
}

Vector3 MyMath::Lerp(const Vector3& v1, const Vector3& v2, float t)
{
	Vector3 result;
	result = Add(v1, Multiply(t, Subtract(v2, v1)));
	return result;
}

Vector3 MyMath::Slerp(const Vector3& vector1, const Vector3& vector2, float t)
{
	// 正規化ベクトルを求める
	Vector3 start = Normalize(vector1);
	Vector3 end = Normalize(vector2);

	// 内積を求める
	float dot = Dot(start, end);
	// 誤差により1.0fを超えるのを防ぐ
	dot = std::clamp(dot, dot, 1.0f);

	// s－九コサインでθの角度を求める
	float theta = std::acosf(dot);

	// θの角度からsinθを求める
	float sinTheta = std::sin(theta);

	// サイン(θ(1-t))を求める
	float sinThetaFrom = std::sin((1 - t) * theta);
	// サインθtを求める
	float sinThetaTo = std::sin(t * theta);

	Vector3 normalizeVector;
	// ゼロ除算を防ぐ
	if (sinTheta < 1.0e-5)
	{
		normalizeVector = start;
	}
	else
	{
		// 球面線形補間したベクトル(単位ベクトル)
		normalizeVector = Add(Multiply(sinThetaFrom / sinTheta, start), Multiply(sinThetaTo / sinTheta, end));
	}

	// ベクトルの長さはstartとendの長さを線形補間
	float length1 = Length(start);
	float length2 = Length(end);
	// Lerpで補間ベクトルの長さを求める
	float length = Lerp(length1, length2, t);

	// 長さを反映
	return Multiply(length, normalizeVector);
}

Vector3 MyMath::Transform(const Vector3& vector, const Matrix4x4& matrix)
{
	Vector3 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + 1.0f * matrix.m[3][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + 1.0f * matrix.m[3][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + 1.0f * matrix.m[3][2];
	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + 1.0f * matrix.m[3][3];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

Vector3 MyMath::TransformNormal(const Vector3& vector, const Matrix4x4& matrix)
{
	Vector3 result;

	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1];
	result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2];

	return result;
}

Vector3 MyMath::Project(const Vector3& v1, const Vector3& v2)
{
	Vector3 c;
	float n = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	float l = powf(sqrtf(powf(v2.x, 2) + powf(v2.y, 2) + powf(v2.z, 2)), 2);
	c.x = n / l * v2.x;
	c.y = n / l * v2.y;
	c.z = n / l * v2.z;
	return c;
}

Vector3 MyMath::ClosestPoint(const Vector3& point, const Segment& segment)
{
	Vector3 cp;
	Vector3 a;
	Vector3 proj;
	a = Subtract(point, segment.origin);
	proj = Project(a, segment.diff);
	cp = {
		segment.origin.x + proj.x,
		segment.origin.y + proj.y,
		segment.origin.z + proj.z
	};
	return cp;
}

Vector3 MyMath::Perpendicular(const Vector3& vector)
{
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y,vector.x,0.0f };
	}
	//法線がz成分のみなら
	return{ 0.0f,-vector.z,vector.y };
}

Vector3 MyMath::ClosestPoint(const AABB& aabb, const Sphere& sphere)
{
	Vector3 closestPoint{
		std::clamp(sphere.center.x,aabb.min.x,aabb.max.x),
		std::clamp(sphere.center.y,aabb.min.y,aabb.max.y),
		std::clamp(sphere.center.z,aabb.min.z,aabb.max.z)
	};
	return closestPoint;
}

Vector3 MyMath::CollisionPoint(const Line& l, const Plane& p)
{
	///衝突している前提の関数
	Vector3 result;
	float dot = Dot(l.diff, p.normal);
	//媒介変数
	float t = (p.distance - Dot(l.origin, p.normal)) / dot;
	result = Add(l.origin, Multiply(t, l.diff));
	return result;
}

Vector3 MyMath::CollisionPoint(const Ray& r, const Plane& p)
{
	///衝突している前提の関数
	Vector3 result;
	float dot = Dot(r.diff, p.normal);
	//媒介変数
	float t = (p.distance - Dot(r.origin, p.normal)) / dot;
	result = Add(r.origin, Multiply(t, r.diff));
	return result;
}

Vector3 MyMath::CollisionPoint(const Segment& s, const Plane& p)
{
	///衝突している前提の関数
	Vector3 result;
	float dot = Dot(s.diff, p.normal);
	//媒介変数
	float t = (p.distance - Dot(s.origin, p.normal)) / dot;
	result = Add(s.origin, Multiply(t, s.diff));
	return result;
}

Vector3 MyMath::Reflect(const Vector3& input, const Vector3& normal)
{
	Vector3 r;
	r = input - 2 * (Dot(input, normal)) * normal;
	return r;
}

Matrix4x4 MyMath::Add(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 c;
	c.m[0][0] = m1.m[0][0] + m2.m[0][0];
	c.m[0][1] = m1.m[0][1] + m2.m[0][1];
	c.m[0][2] = m1.m[0][2] + m2.m[0][2];
	c.m[0][3] = m1.m[0][3] + m2.m[0][3];
	c.m[1][0] = m1.m[1][0] + m2.m[1][0];
	c.m[1][1] = m1.m[1][1] + m2.m[1][1];
	c.m[1][2] = m1.m[1][2] + m2.m[1][2];
	c.m[1][3] = m1.m[1][3] + m2.m[1][3];
	c.m[2][0] = m1.m[2][0] + m2.m[2][0];
	c.m[2][1] = m1.m[2][1] + m2.m[2][1];
	c.m[2][2] = m1.m[2][2] + m2.m[2][2];
	c.m[2][3] = m1.m[2][3] + m2.m[2][3];
	c.m[3][0] = m1.m[3][0] + m2.m[3][0];
	c.m[3][1] = m1.m[3][1] + m2.m[3][1];
	c.m[3][2] = m1.m[3][2] + m2.m[3][2];
	c.m[3][3] = m1.m[3][3] + m2.m[3][3];
	return c;
}

Matrix4x4 MyMath::Subtract(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 c;
	c.m[0][0] = m1.m[0][0] - m2.m[0][0];
	c.m[0][1] = m1.m[0][1] - m2.m[0][1];
	c.m[0][2] = m1.m[0][2] - m2.m[0][2];
	c.m[0][3] = m1.m[0][3] - m2.m[0][3];
	c.m[1][0] = m1.m[1][0] - m2.m[1][0];
	c.m[1][1] = m1.m[1][1] - m2.m[1][1];
	c.m[1][2] = m1.m[1][2] - m2.m[1][2];
	c.m[1][3] = m1.m[1][3] - m2.m[1][3];
	c.m[2][0] = m1.m[2][0] - m2.m[2][0];
	c.m[2][1] = m1.m[2][1] - m2.m[2][1];
	c.m[2][2] = m1.m[2][2] - m2.m[2][2];
	c.m[2][3] = m1.m[2][3] - m2.m[2][3];
	c.m[3][0] = m1.m[3][0] - m2.m[3][0];
	c.m[3][1] = m1.m[3][1] - m2.m[3][1];
	c.m[3][2] = m1.m[3][2] - m2.m[3][2];
	c.m[3][3] = m1.m[3][3] - m2.m[3][3];
	return c;
}

Matrix4x4 MyMath::Multiply(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 c;
	c.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
	c.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
	c.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
	c.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];
	c.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
	c.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
	c.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
	c.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];
	c.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
	c.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
	c.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
	c.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];
	c.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
	c.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
	c.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
	c.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];
	return c;
}

Matrix4x4 MyMath::Inverse(const Matrix4x4& m)
{
	Matrix4x4 c;
	float A;
	A = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2] -
		m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2] -
		m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2] +
		m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2] +
		m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2] -
		m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2] -
		m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0] +
		m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

	c.m[0][0] = (1.0f / A) * (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] + m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] - m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]);
	c.m[0][1] = (1.0f / A) * (-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] - m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][3] * m.m[2][2] * m.m[3][1] + m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]);
	c.m[0][2] = (1.0f / A) * (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] + m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] - m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]);
	c.m[0][3] = (1.0f / A) * (-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] - m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][3] * m.m[1][2] * m.m[2][1] + m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]);
	c.m[1][0] = (1.0f / A) * (-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] - m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][3] * m.m[2][2] * m.m[3][0] + m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]);
	c.m[1][1] = (1.0f / A) * (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] + m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] - m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]);
	c.m[1][2] = (1.0f / A) * (-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] - m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][3] * m.m[1][2] * m.m[3][0] + m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]);
	c.m[1][3] = (1.0f / A) * (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] + m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] - m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]);
	c.m[2][0] = (1.0f / A) * (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] + m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] - m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]);
	c.m[2][1] = (1.0f / A) * (-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] - m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][0] + m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]);
	c.m[2][2] = (1.0f / A) * (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] + m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] - m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]);
	c.m[2][3] = (1.0f / A) * (-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] - m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] + m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]);
	c.m[3][0] = (1.0f / A) * (-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] - m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][2] * m.m[2][1] * m.m[3][0] + m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]);
	c.m[3][1] = (1.0f / A) * (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] + m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] - m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]);
	c.m[3][2] = (1.0f / A) * (-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] - m.m[0][2] * m.m[1][0] * m.m[3][1] + m.m[0][2] * m.m[1][1] * m.m[3][0] + m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]);
	c.m[3][3] = (1.0f / A) * (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] + m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] - m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]);

	return c;
}

Matrix4x4 MyMath::Transpose(const Matrix4x4& m)
{
	Matrix4x4 c;
	c.m[0][0] = m.m[0][0];
	c.m[0][1] = m.m[1][0];
	c.m[0][2] = m.m[2][0];
	c.m[0][3] = m.m[3][0];
	c.m[1][0] = m.m[0][1];
	c.m[1][1] = m.m[1][1];
	c.m[1][2] = m.m[2][1];
	c.m[1][3] = m.m[3][1];
	c.m[2][0] = m.m[0][2];
	c.m[2][1] = m.m[1][2];
	c.m[2][2] = m.m[2][2];
	c.m[2][3] = m.m[3][2];
	c.m[3][0] = m.m[0][3];
	c.m[3][1] = m.m[1][3];
	c.m[3][2] = m.m[2][3];
	c.m[3][3] = m.m[3][3];
	return c;
}

Matrix4x4 MyMath::MakeIdentity4x4()
{
	Matrix4x4 c;
	c.m[0][0] = 1.0f;
	c.m[0][1] = 0.0f;
	c.m[0][2] = 0.0f;
	c.m[0][3] = 0.0f;
	c.m[1][0] = 0.0f;
	c.m[1][1] = 1.0f;
	c.m[1][2] = 0.0f;
	c.m[1][3] = 0.0f;
	c.m[2][0] = 0.0f;
	c.m[2][1] = 0.0f;
	c.m[2][2] = 1.0f;
	c.m[2][3] = 0.0f;
	c.m[3][0] = 0.0f;
	c.m[3][1] = 0.0f;
	c.m[3][2] = 0.0f;
	c.m[3][3] = 1.0f;
	return c;
}

Matrix4x4 MyMath::MakeTranslateMatrix(const Vector3& translate)
{
	Matrix4x4 c;
	c.m[0][0] = 1;
	c.m[0][1] = 0;
	c.m[0][2] = 0;
	c.m[0][3] = 0;
	c.m[1][0] = 0;
	c.m[1][1] = 1;
	c.m[1][2] = 0;
	c.m[1][3] = 0;
	c.m[2][0] = 0;
	c.m[2][1] = 0;
	c.m[2][2] = 1;
	c.m[2][3] = 0;
	c.m[3][0] = translate.x;
	c.m[3][1] = translate.y;
	c.m[3][2] = translate.z;
	c.m[3][3] = 1;
	return c;
}

Matrix4x4 MyMath::MakeScaleMatrix(const Vector3& scale)
{
	Matrix4x4 c;
	c.m[0][0] = scale.x;
	c.m[0][1] = 0;
	c.m[0][2] = 0;
	c.m[0][3] = 0;
	c.m[1][0] = 0;
	c.m[1][1] = scale.y;
	c.m[1][2] = 0;
	c.m[1][3] = 0;
	c.m[2][0] = 0;
	c.m[2][1] = 0;
	c.m[2][2] = scale.z;
	c.m[2][3] = 0;
	c.m[3][0] = 0;
	c.m[3][1] = 0;
	c.m[3][2] = 0;
	c.m[3][3] = 1;
	return c;
}

Matrix4x4 MyMath::MakeRotateXMatrix(float radian)
{
	Matrix4x4 c;
	c.m[0][0] = 1;
	c.m[0][1] = 0;
	c.m[0][2] = 0;
	c.m[0][3] = 0;
	c.m[1][0] = 0;
	c.m[1][1] = std::cos(radian);
	c.m[1][2] = std::sin(radian);
	c.m[1][3] = 0;
	c.m[2][0] = 0;
	c.m[2][1] = -std::sin(radian);
	c.m[2][2] = std::cos(radian);
	c.m[2][3] = 0;
	c.m[3][0] = 0;
	c.m[3][1] = 0;
	c.m[3][2] = 0;
	c.m[3][3] = 1;
	return c;
}

Matrix4x4 MyMath::MakeRotateYMatrix(float radian)
{
	Matrix4x4 c;
	c.m[0][0] = std::cos(radian);
	c.m[0][1] = 0;
	c.m[0][2] = -std::sin(radian);
	c.m[0][3] = 0;
	c.m[1][0] = 0;
	c.m[1][1] = 1;
	c.m[1][2] = 0;
	c.m[1][3] = 0;
	c.m[2][0] = std::sin(radian);
	c.m[2][1] = 0;
	c.m[2][2] = std::cos(radian);
	c.m[2][3] = 0;
	c.m[3][0] = 0;
	c.m[3][1] = 0;
	c.m[3][2] = 0;
	c.m[3][3] = 1;
	return c;
}

Matrix4x4 MyMath::MakeRotateZMatrix(float radian)
{
	Matrix4x4 c;
	c.m[0][0] = std::cos(radian);
	c.m[0][1] = std::sin(radian);
	c.m[0][2] = 0;
	c.m[0][3] = 0;
	c.m[1][0] = -std::sin(radian);
	c.m[1][1] = std::cos(radian);
	c.m[1][2] = 0;
	c.m[1][3] = 0;
	c.m[2][0] = 0;
	c.m[2][1] = 0;
	c.m[2][2] = 1;
	c.m[2][3] = 0;
	c.m[3][0] = 0;
	c.m[3][1] = 0;
	c.m[3][2] = 0;
	c.m[3][3] = 1;
	return c;
}

Matrix4x4 MyMath::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate)
{
	Matrix4x4 c;
	//回転行列統合
	Matrix4x4 rx = MakeRotateXMatrix(rotate.x);
	Matrix4x4 ry = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rz = MakeRotateZMatrix(rotate.z);
	Matrix4x4 rxyz = Multiply(rx, Multiply(ry, rz));

	c.m[0][0] = scale.x * rxyz.m[0][0];
	c.m[0][1] = scale.x * rxyz.m[0][1];
	c.m[0][2] = scale.x * rxyz.m[0][2];
	c.m[0][3] = 0;
	c.m[1][0] = scale.y * rxyz.m[1][0];
	c.m[1][1] = scale.y * rxyz.m[1][1];
	c.m[1][2] = scale.y * rxyz.m[1][2];
	c.m[1][3] = 0;
	c.m[2][0] = scale.z * rxyz.m[2][0];
	c.m[2][1] = scale.z * rxyz.m[2][1];
	c.m[2][2] = scale.z * rxyz.m[2][2];
	c.m[2][3] = 0;
	c.m[3][0] = translate.x;
	c.m[3][1] = translate.y;
	c.m[3][2] = translate.z;
	c.m[3][3] = 1;

	return c;
}

Matrix4x4 MyMath::MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate)
{
	Matrix4x4 c;

	// クォータニオンから回転行列を生成
	Matrix4x4 rotationMatrix = MakeRotateMatrix(rotate);

	// スケールの適用
	c.m[0][0] = scale.x * rotationMatrix.m[0][0];
	c.m[0][1] = scale.x * rotationMatrix.m[0][1];
	c.m[0][2] = scale.x * rotationMatrix.m[0][2];
	c.m[0][3] = 0;

	c.m[1][0] = scale.y * rotationMatrix.m[1][0];
	c.m[1][1] = scale.y * rotationMatrix.m[1][1];
	c.m[1][2] = scale.y * rotationMatrix.m[1][2];
	c.m[1][3] = 0;

	c.m[2][0] = scale.z * rotationMatrix.m[2][0];
	c.m[2][1] = scale.z * rotationMatrix.m[2][1];
	c.m[2][2] = scale.z * rotationMatrix.m[2][2];
	c.m[2][3] = 0;

	// 平行移動の適用
	c.m[3][0] = translate.x;
	c.m[3][1] = translate.y;
	c.m[3][2] = translate.z;
	c.m[3][3] = 1;

	return c;
}

Matrix4x4 MyMath::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
{
	Matrix4x4 c;
	c.m[0][0] = (1 / aspectRatio) * Cot(fovY / 2);
	c.m[0][1] = 0;
	c.m[0][2] = 0;
	c.m[0][3] = 0;
	c.m[1][0] = 0;
	c.m[1][1] = Cot(fovY / 2);
	c.m[1][2] = 0;
	c.m[1][3] = 0;
	c.m[2][0] = 0;
	c.m[2][1] = 0;
	c.m[2][2] = farClip / (farClip - nearClip);
	c.m[2][3] = 1;
	c.m[3][0] = 0;
	c.m[3][1] = 0;
	c.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	c.m[3][3] = 0;
	return c;
}

Matrix4x4 MyMath::MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip)
{
	Matrix4x4 c;
	c.m[0][0] = 2 / (right - left);
	c.m[0][1] = 0;
	c.m[0][2] = 0;
	c.m[0][3] = 0;
	c.m[1][0] = 0;
	c.m[1][1] = 2 / (top - bottom);
	c.m[1][2] = 0;
	c.m[1][3] = 0;
	c.m[2][0] = 0;
	c.m[2][1] = 0;
	c.m[2][2] = 1 / (farClip - nearClip);
	c.m[2][3] = 0;
	c.m[3][0] = (left + right) / (left - right);
	c.m[3][1] = (top + bottom) / (bottom - top);
	c.m[3][2] = nearClip / (nearClip - farClip);
	c.m[3][3] = 1;
	return c;
}

Matrix4x4 MyMath::MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth)
{
	Matrix4x4 c;
	c.m[0][0] = width / 2;
	c.m[0][1] = 0;
	c.m[0][2] = 0;
	c.m[0][3] = 0;
	c.m[1][0] = 0;
	c.m[1][1] = -height / 2;
	c.m[1][2] = 0;
	c.m[1][3] = 0;
	c.m[2][0] = 0;
	c.m[2][1] = 0;
	c.m[2][2] = maxDepth - minDepth;
	c.m[2][3] = 0;
	c.m[3][0] = left + (width / 2);
	c.m[3][1] = top + (height / 2);
	c.m[3][2] = minDepth;
	c.m[3][3] = 1;
	return c;
}

Matrix4x4 MyMath::CreateRotationFromEulerAngles(float pitch, float yaw, float roll)
{
	// ピッチ（X軸回転）、ヨー（Y軸回転）、ロール（Z軸回転）の角度をラジアンに変換
	float cosPitch = cosf(pitch);
	float sinPitch = sinf(pitch);
	float cosYaw = cosf(yaw);
	float sinYaw = sinf(yaw);
	float cosRoll = cosf(roll);
	float sinRoll = sinf(roll);

	// X軸回転行列
	Matrix4x4 rotationX = {
		1, 0, 0, 0,
		0, cosPitch, -sinPitch, 0,
		0, sinPitch, cosPitch, 0,
		0, 0, 0, 1
	};

	// Y軸回転行列
	Matrix4x4 rotationY = {
		cosYaw, 0, sinYaw, 0,
		0, 1, 0, 0,
		-sinYaw, 0, cosYaw, 0,
		0, 0, 0, 1
	};

	// Z軸回転行列
	Matrix4x4 rotationZ = {
		cosRoll, -sinRoll, 0, 0,
		sinRoll, cosRoll, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	// 回転行列を合成する（順番重要）
	Matrix4x4 rotationMatrix = rotationZ * rotationY * rotationX;

	return rotationMatrix;
}

Matrix4x4 MyMath::LookAt(Vector3 eye, Vector3 target, Vector3 up)
{

	// 前方向ベクトル（正規化）
	Vector3 forward = Normalize(target - eye);

	// 右方向ベクトル（正規化）
	Vector3 right = Normalize(Cross(up, forward));

	// 上方向ベクトル（修正済み）
	Vector3 upCorrected = Cross(forward, right);

	// ビュー行列を構成
	Matrix4x4 viewMatrix = {
		right.x, upCorrected.x, -forward.x, 0.0f,
		right.y, upCorrected.y, -forward.y, 0.0f,
		right.z, upCorrected.z, -forward.z, 0.0f,
		-Dot(right, eye), -Dot(upCorrected, eye), Dot(forward, eye), 1.0f
	};

	return viewMatrix;

}

Quaternion MyMath::Add(const Quaternion& q1, const Quaternion& q2) {
	return Quaternion(q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w);
}

Quaternion MyMath::Subtract(const Quaternion& q1, const Quaternion& q2) {
	return Quaternion(q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w);
}

Quaternion MyMath::Multiply(const Quaternion& q1, const Quaternion& q2) {
	Quaternion c;
	//各Quaternoinのxyz成分をVector3に直す
	Vector3 q1v = { q1.x,q1.y ,q1.z };
	Vector3 q2v = { q2.x,q2.y ,q2.z };
	//演算
	c.w = q1.w * q2.w - Dot(q1v, q2v);
	c.x = Vector3(Cross(q1v, q2v) + (q2.w * q1v) + (q1.w * q2v)).x;
	c.y = Vector3(Cross(q1v, q2v) + (q2.w * q1v) + (q1.w * q2v)).y;
	c.z = Vector3(Cross(q1v, q2v) + (q2.w * q1v) + (q1.w * q2v)).z;

	return c;
}

Quaternion MyMath::Multiply(float scalar, const Quaternion& q)
{
	return { q.x * scalar,q.y * scalar, q.z * scalar, q.w * scalar };
}

float MyMath::Dot(const Quaternion& q1, const Quaternion& q2)
{
	return q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
}

float MyMath::Norm(const Quaternion& q) {
	return std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

Quaternion MyMath::Normalize(const Quaternion& q) {
	float n = Norm(q);
	if (n == 0.0f) {
		throw std::runtime_error("Zero norm quaternion cannot be normalized");
	}
	return Quaternion(q.x / n, q.y / n, q.z / n, q.w / n);
}

Quaternion MyMath::Conjugate(const Quaternion& q) {
	return Quaternion(-q.x, -q.y, -q.z, q.w);
}

Quaternion MyMath::Inverse(const Quaternion& q) {
	float normSquared = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
	if (normSquared == 0.0f) {
		throw std::runtime_error("Cannot invert a quaternion with zero norm");
	}
	Quaternion conjugate = Conjugate(q);
	return Quaternion(conjugate.x / normSquared, conjugate.y / normSquared,
		conjugate.z / normSquared, conjugate.w / normSquared);
}

Quaternion MyMath::MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	float halfAngle = angle * 0.5f;
	float sinHalfAngle = sin(halfAngle);
	return Quaternion(
		cos(halfAngle),
		axis.x * sinHalfAngle,
		axis.y * sinHalfAngle,
		axis.z * sinHalfAngle
	);
}

Vector3 MyMath::RotateVector(const Vector3& vector, const Quaternion& quaternion)
{
	Vector3 c;
	Quaternion vtq = { vector.x,vector.y, vector.z, 0.0f };
	Quaternion result = quaternion * vtq * Inverse(quaternion);
	c.x = result.x;
	c.y = result.y;
	c.z = result.z;
	return c;
}

Matrix4x4 MyMath::MakeRotateMatrix(const Quaternion& q) {
	Matrix4x4 c;
	c.m[0][0] = std::powf(q.w, 2) + std::powf(q.x, 2) - std::powf(q.y, 2) - std::powf(q.z, 2);
	c.m[0][1] = 2.0f * (q.x * q.y + q.w * q.z);
	c.m[0][2] = 2.0f * (q.x * q.z - q.w * q.y);
	c.m[0][3] = 0.0f;
	c.m[1][0] = 2.0f * (q.x * q.y - q.w * q.z);
	c.m[1][1] = std::powf(q.w, 2) - std::powf(q.x, 2) + std::powf(q.y, 2) - std::powf(q.z, 2);
	c.m[1][2] = 2.0f * (q.y * q.z + q.w * q.x);
	c.m[1][3] = 0.0f;
	c.m[2][0] = 2.0f * (q.x * q.z + q.w * q.y);
	c.m[2][1] = 2.0f * (q.y * q.z - q.w * q.x);
	c.m[2][2] = std::powf(q.w, 2) - std::powf(q.x, 2) - std::powf(q.y, 2) + std::powf(q.z, 2);
	c.m[2][3] = 0.0f;
	c.m[3][0] = 0.0f;
	c.m[3][1] = 0.0f;
	c.m[3][2] = 0.0f;
	c.m[3][3] = 1.0f;
	return c;
}

Quaternion MyMath::FromEulerAngles(Vector3 euler) {
	float cy = std::cos(euler.x * 0.5f);
	float sy = std::sin(euler.x * 0.5f);
	float cp = std::cos(euler.y * 0.5f);
	float sp = std::sin(euler.y * 0.5f);
	float cr = std::cos(euler.z * 0.5f);
	float sr = std::sin(euler.z * 0.5f);

	return Quaternion(
		sr * cp * cy - cr * sp * sy,
		cr * sp * cy + sr * cp * sy,
		cr * cp * sy - sr * sp * cy,
		cr * cp * cy + sr * sp * sy
	);
}

Vector3 MyMath::ToEulerAngles(const Quaternion& q) {
	Vector3 euler;

	// ピッチ（x軸回り）
	float sinp = 2.0f * (q.w * q.x + q.y * q.z);
	if (std::abs(sinp) >= 1)
		euler.x = std::copysign(std::numbers::pi_v<float> / 2.0f, sinp); // ピッチを -90 〜 90 度にクランプ
	else
		euler.x = std::asin(sinp);

	// ヨー（y軸回り）
	float siny_cosy = 2.0f * (q.w * q.y - q.z * q.x);
	float cosy_cosy = 1.0f - 2.0f * (q.y * q.y + q.x * q.x);
	euler.y = std::atan2(siny_cosy, cosy_cosy);

	// ロール（z軸回り）
	float sinr_cosr = 2.0f * (q.w * q.z + q.x * q.y);
	float cosr_cosr = 1.0f - 2.0f * (q.z * q.z + q.x * q.x);
	euler.z = std::atan2(sinr_cosr, cosr_cosr);

	return euler;
}

Quaternion MyMath::Slerp(const Quaternion& q0, const Quaternion& q1, float t)
{
	Quaternion c;
	Quaternion q0c = q0;
	Quaternion q1c = q1;
	//q0とq1の内積
	float dot = Dot(q0, q1);
	if (dot < 0.0f) {
		//もう片方の回転を利用する
		q0c = -q0c;
		//内積も反転
		dot = -dot;
	}
	//内積が1に近い場合、線形補完を使用
	const float EPSILON = 5e-4f;//0.0005
	if (dot >= 1.0f - EPSILON) {
		c = q0c * (1.0f - t) + q1c * t;
		return c;
	}

	//なす角を求める
	float theta = std::acosf(dot);
	//thetaとsinを使って補間係数scale0,scale1を求める
	float sin_theta = std::sqrt(1.0f - dot * dot);
	float scale0 = std::sin((1.0f - t) * theta) / sin_theta;
	float scale1 = std::sin(t * theta) / sin_theta;
	//補間
	c = scale0 * q0c + scale1 * q1c;

	return c;
}

float MyMath::Cot(float rad)
{
	float c;

	c = 1 / std::tan(rad);

	return c;
}

float MyMath::Length(const Vector3& v)
{
	float c;
	c = std::sqrt(static_cast<float>(std::pow(v.x, 2) + std::pow(v.y, 2) + std::pow(v.z, 2)));
	return c;
}

float MyMath::Dot(const Vector3& v1, const Vector3& v2)
{
	float c;
	c = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return c;
}

float MyMath::Lerp(float s1, float s2, float t)
{
	return s1 * (1 - t) + s2 * t;
}

std::pair<float, float> MyMath::ProjectOntoAxis(const Vector3* vertices, int count, const Vector3& axis)
{
	float min = Dot(vertices[0], axis);
	float max = min;
	for (int i = 1; i < count; ++i) {
		float projection = Dot(vertices[i], axis);
		min = min(min, projection);
		max = max(max, projection);
	}
	return { min, max };
}

float MyMath::EaseInSine(float ratio)
{
	return float(1 - std::cos((ratio * std::numbers::pi_v<float>) / 2));
}

float MyMath::EaseOutSine(float ratio)
{
	return float(std::sin((ratio * std::numbers::pi_v<float>) / 2));
}

float MyMath::EaseInOutSine(float ratio)
{
	return float(-(std::cos(std::numbers::pi_v<float> *ratio) - 1) / 2);
}

float MyMath::EaseInCubic(float ratio)
{
	return float(ratio * ratio * ratio);
}

float MyMath::EaseOutCubic(float ratio)
{
	return float(1 - std::pow(1 - ratio, 3));
}

float MyMath::EaseInOutCubic(float ratio)
{
	return float(ratio < 0.5 ? 4 * ratio * ratio * ratio : 1 - std::pow(-2 * ratio + 2, 3) / 2);
}

float MyMath::EaseInQuint(float ratio)
{
	return float(ratio * ratio * ratio * ratio * ratio);
}

float MyMath::EaseOutQuint(float ratio)
{
	return float(1 - std::pow(1 - ratio, 5));
}

float MyMath::EaseInOutQuint(float ratio)
{
	return float(ratio < 0.5 ? 16 * ratio * ratio * ratio * ratio * ratio : 1 - std::pow(-2 * ratio + 2, 5) / 2);
}

float MyMath::EaseInCirc(float ratio)
{
	return float(1 - std::sqrt(1 - std::pow(ratio, 2)));
}

float MyMath::EaseOutCirc(float ratio)
{
	return float(std::sqrt(1 - std::pow(ratio - 1, 2)));
}

float MyMath::EaseInOutCirc(float ratio)
{
	return float(ratio < 0.5
		? (1 - std::sqrt(1 - std::pow(2 * ratio, 2))) / 2
		: (std::sqrt(1 - std::pow(-2 * ratio + 2, 2)) + 1) / 2);
}

float MyMath::EaseInElastic(float ratio)
{
	const float c4 = (2 * std::numbers::pi_v<float>) / 3;

	return float(ratio == 0
		? 0
		: ratio == 1
		? 1
		: -std::pow(2, 10 * ratio - 10) * std::sin((ratio * 10 - 10.75) * c4));
}

float MyMath::EaseOutElastic(float ratio)
{
	const float c4 = (2 * std::numbers::pi_v<float>) / 3;

	return float(ratio == 0
		? 0
		: ratio == 1
		? 1
		: std::pow(2, -10 * ratio) * std::sin((ratio * 10 - 0.75) * c4) + 1);
}

float MyMath::EaseInOutElastic(float ratio)
{
	const float c5 = (2 * std::numbers::pi_v<float>) / 4.5f;

	return float(ratio == 0
		? 0
		: ratio == 1
		? 1
		: ratio < 0.5
		? -(std::pow(2, 20 * ratio - 10) * std::sin((20 * ratio - 11.125) * c5)) / 2
		: (std::pow(2, -20 * ratio + 10) * std::sin((20 * ratio - 11.125) * c5)) / 2 + 1);
}

float MyMath::EaseInQuad(float ratio)
{
	return float(ratio * ratio);
}

float MyMath::EaseOutQuad(float ratio)
{
	return float(1 - (1 - ratio) * (1 - ratio));
}

float MyMath::EaseInOutQuad(float ratio)
{
	return float(ratio < 0.5 ? 2 * ratio * ratio : 1 - std::pow(-2 * ratio + 2, 2) / 2);
}

float MyMath::EaseInQuart(float ratio)
{
	return float(ratio * ratio * ratio * ratio);
}

float MyMath::EaseOutQuart(float ratio)
{
	return float(1 - std::pow(1 - ratio, 4));
}

float MyMath::EaseInOutQuart(float ratio)
{
	return float(ratio < 0.5 ? 8 * ratio * ratio * ratio * ratio : 1 - std::pow(-2 * ratio + 2, 4) / 2);
}

float MyMath::EaseInExpo(float ratio)
{
	return float(ratio == 0 ? 0 : std::pow(2, 10 * ratio - 10));
}

float MyMath::EaseOutExpo(float ratio)
{
	return float(ratio == 1 ? 1 : 1 - std::pow(2, -10 * ratio));
}

float MyMath::EaseInOutExpo(float ratio)
{
	return float(ratio == 0
		? 0
		: ratio == 1
		? 1
		: ratio < 0.5 ? std::pow(2, 20 * ratio - 10) / 2
		: (2 - std::pow(2, -20 * ratio + 10)) / 2);
}

float MyMath::EaseInBack(float ratio)
{
	const float c1 = 1.70158f;
	const float c3 = c1 + 1;

	return float(c3 * ratio * ratio * ratio - c1 * ratio * ratio);
}

float MyMath::EaseOutBack(float ratio)
{
	const float c1 = 1.70158f;
	const float c3 = c1 + 1;

	return float(1 + c3 * std::pow(ratio - 1, 3) + c1 * std::pow(ratio - 1, 2));
}

float MyMath::EaseInOutBack(float ratio)
{
	const float c1 = 1.70158f;
	const float c2 = c1 * 1.525f;

	return float(ratio < 0.5
		? (std::pow(2 * ratio, 2) * ((c2 + 1) * 2 * ratio - c2)) / 2
		: (std::pow(2 * ratio - 2, 2) * ((c2 + 1) * (ratio * 2 - 2) + c2) + 2) / 2);
}

float MyMath::EaseInBounce(float ratio)
{
	return float(1 - EaseOutBounce(1 - ratio));
}

float MyMath::EaseOutBounce(float ratio)
{
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (ratio < 1 / d1) {
		return float(n1 * ratio * ratio);
	}
	else if (ratio < 2 / d1) {
		return float(n1 * (ratio -= 1.5f / d1) * ratio + 0.75);
	}
	else if (ratio < 2.5 / d1) {
		return float(n1 * (ratio -= 2.25f / d1) * ratio + 0.9375);
	}
	else {
		return float(n1 * (ratio -= 2.625f / d1) * ratio + 0.984375);
	}
}

float MyMath::EaseInOutBounce(float ratio)
{
	return float(ratio < 0.5
		? (1 - EaseOutBounce(1 - 2 * ratio)) / 2
		: (1 + EaseOutBounce(2 * ratio - 1)) / 2);
}

bool MyMath::IsCollision(const Sphere& s1, const Sphere& s2)
{
	//2つの球の中心点間の距離を求める
	float distance = Length(Subtract(s2.center, s1.center));
	//半径の合計よりも短ければ衝突
	if (distance <= s1.radius + s2.radius) {
		return true;
	}
	else {
		return false;
	}
}

bool MyMath::IsCollision(const Plane& plane, const Sphere& sphere)
{
	//球の中心と平面との距離を計算
	float distance;
	float k;
	k = sqrtf(powf(Dot(plane.normal, sphere.center) - plane.distance, 2));
	Vector3 q;//球の中心から平面に垂直に線を下したときに交わる点
	q = Subtract(sphere.center, Multiply(k, plane.normal));
	distance = Length(Subtract(sphere.center, q));
	//衝突判定
	if (distance <= sphere.radius) {
		return true;
	}
	else {
		return false;
	}
}

bool MyMath::IsCollision(const Line& line, const Plane& plane)
{
	//垂直判定を求める
	float dot = Dot(plane.normal, line.diff);
	//平行の場合衝突していない。
	if (dot == 0.0f) {
		return false;
	}
	//tを求める
	float t = (plane.distance - Dot(line.origin, plane.normal)) / dot;
	//直線の当たり判定
	if (t >= 0 || t < 0) {
		return true;
	}
	else {
		return false;
	}
}

bool MyMath::IsCollision(const Ray& ray, const Plane& plane)
{
	//垂直判定を求める
	float dot = Dot(plane.normal, ray.diff);
	//平行の場合衝突していない。
	if (dot == 0.0f) {
		return false;
	}
	//tを求める
	float t = (plane.distance - Dot(ray.origin, plane.normal)) / dot;
	//半直線の当たり判定
	if (t >= 0) {
		return true;
	}
	else {
		return false;
	}
}

bool MyMath::IsCollision(const Segment& segment, const Plane& plane)
{
	//平行判定
	float dot = Dot(segment.diff, plane.normal);
	if (dot == 0.0f) {
		return false;
	}

	//媒介変数
	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;

	//当たり判定結果
	if (t >= 0 && t <= 1) {
		return true;
	}
	else {
		return false;
	}
}

bool MyMath::IsCollision(const Segment& segment, const Triangle& triangle)
{
	//三角形のある面を作る
	Plane plane;
	//三角形の座標から、法線nを求める
	Vector3 vv1, vv2, n;
	vv1 = Subtract(triangle.vertices[1], triangle.vertices[0]);
	vv2 = Subtract(triangle.vertices[2], triangle.vertices[1]);
	n = Normalize(Cross(vv1, vv2));
	//ディスタンスを求める
	float d = Dot(triangle.vertices[0], n);
	//面に変換
	plane.normal = n;
	plane.distance = d;

	//面と線の当たり判定
	//接触点Pを求める
	float dot = Dot(segment.diff, plane.normal);
	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;
	Vector3 p = Add(segment.origin, Multiply(t, segment.diff));
	//当たり判定
	if (IsCollision(segment, plane)) {
		//必要なベクトル
		Vector3 v01 = Subtract(triangle.vertices[1], triangle.vertices[0]);
		Vector3 v12 = Subtract(triangle.vertices[2], triangle.vertices[1]);
		Vector3 v20 = Subtract(triangle.vertices[0], triangle.vertices[2]);
		Vector3 v0p = Subtract(p, triangle.vertices[0]);
		Vector3 v1p = Subtract(p, triangle.vertices[1]);
		Vector3 v2p = Subtract(p, triangle.vertices[2]);
		//各辺を結んだベクトルと、頂点の衝突点pを結んだベクトルのクロス積を取る
		Vector3 cross01 = Cross(v01, v1p);
		Vector3 cross12 = Cross(v12, v2p);
		Vector3 cross20 = Cross(v20, v0p);
		//全ての小三角形のクロス積と法線が同じ方向を向いていたら衝突
		if (Dot(cross01, plane.normal) >= 0.0f &&
			Dot(cross12, plane.normal) >= 0.0f &&
			Dot(cross20, plane.normal) >= 0.0f) {
			//衝突
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

bool MyMath::IsCollision(const AABB& a, const AABB& b)
{
	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z)) {
		//衝突
		return true;
	}
	else {
		return false;
	}
}

bool MyMath::IsCollision(const AABB& aabb, const Vector3& point)
{
	//点の全ての座標成分がAABB内にあるなら衝突
	if ((point.x >= aabb.min.x && point.x <= aabb.max.x) &&
		(point.y >= aabb.min.y && point.y <= aabb.max.y) &&
		(point.z >= aabb.min.z && point.z <= aabb.max.z)) {
		return true;
	}
	return false;
}

bool MyMath::IsCollision(const AABB& aabb, const Sphere& sphere)
{
	//球の中心とAABBとの最近接点を求める
	Vector3 closestPoint = ClosestPoint(aabb, sphere);
	//最近接点と球の中心の距離を求める
	float distance = Length(Subtract(closestPoint, sphere.center));
	//距離が平均よりも小さければ衝突
	if (distance <= sphere.radius) {
		return true;
	}
	else {
		return false;
	}
}

bool MyMath::IsCollision(const AABB& aabb, const Line& line)
{
	//segmentの成分が全て0(点)の場合エラー
	if (line.diff.x == 0 && line.diff.y == 0 && line.diff.z == 0) {
		assert("線の成分が全て0");
	}

	//6つの平面を構造体に入れる
	Plane pxmin;
	Plane pxmax;
	Plane pymin;
	Plane pymax;
	Plane pzmin;
	Plane pzmax;
	//法線の値を入力
	pxmin.normal = { 1,0,0 };
	pxmax.normal = { 1,0,0 };
	pymin.normal = { 0,1,0 };
	pymax.normal = { 0,1,0 };
	pzmin.normal = { 0,0,1 };
	pzmax.normal = { 0,0,1 };
	//距離の値を入力
	pxmin.distance = Dot(aabb.min, pxmin.normal);
	pymin.distance = Dot(aabb.min, pymin.normal);
	pzmin.distance = Dot(aabb.min, pzmin.normal);
	pxmax.distance = Dot(aabb.max, pxmax.normal);
	pymax.distance = Dot(aabb.max, pymax.normal);
	pzmax.distance = Dot(aabb.max, pzmax.normal);
	//それぞれの平面と線分の衝突点を求める
	if (IsCollision(line, pxmin)) {
		Vector3 cp = CollisionPoint(line, pxmin);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(line, pxmax)) {
		Vector3 cp = CollisionPoint(line, pxmax);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(line, pymin)) {
		Vector3 cp = CollisionPoint(line, pymin);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(line, pymax)) {
		Vector3 cp = CollisionPoint(line, pymax);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(line, pzmin)) {
		Vector3 cp = CollisionPoint(line, pzmin);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(line, pzmax)) {
		Vector3 cp = CollisionPoint(line, pzmax);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	//これ以上衝突条件は無い
	return false;
}

bool MyMath::IsCollision(const AABB& aabb, const Ray& ray)
{
	//segmentの成分が全て0(点)の場合エラー
	if (ray.diff.x == 0 && ray.diff.y == 0 && ray.diff.z == 0) {
		assert("線の成分が全て0");
	}

	//6つの平面を構造体に入れる
	Plane pxmin;
	Plane pxmax;
	Plane pymin;
	Plane pymax;
	Plane pzmin;
	Plane pzmax;
	//法線の値を入力
	pxmin.normal = { 1,0,0 };
	pxmax.normal = { 1,0,0 };
	pymin.normal = { 0,1,0 };
	pymax.normal = { 0,1,0 };
	pzmin.normal = { 0,0,1 };
	pzmax.normal = { 0,0,1 };
	//距離の値を入力
	pxmin.distance = Dot(aabb.min, pxmin.normal);
	pymin.distance = Dot(aabb.min, pymin.normal);
	pzmin.distance = Dot(aabb.min, pzmin.normal);
	pxmax.distance = Dot(aabb.max, pxmax.normal);
	pymax.distance = Dot(aabb.max, pymax.normal);
	pzmax.distance = Dot(aabb.max, pzmax.normal);
	//それぞれの平面と線分の衝突点を求める
	if (IsCollision(ray, pxmin)) {
		Vector3 cp = CollisionPoint(ray, pxmin);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(ray, pxmax)) {
		Vector3 cp = CollisionPoint(ray, pxmax);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(ray, pymin)) {
		Vector3 cp = CollisionPoint(ray, pymin);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(ray, pymax)) {
		Vector3 cp = CollisionPoint(ray, pymax);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(ray, pzmin)) {
		Vector3 cp = CollisionPoint(ray, pzmin);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(ray, pzmax)) {
		Vector3 cp = CollisionPoint(ray, pzmax);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	//これ以上衝突条件は無い
	return false;
}

bool MyMath::IsCollision(const AABB& aabb, const Segment& segment)
{
	//segmentの成分が全て0(点)の場合エラー
	if (segment.diff.x == 0 && segment.diff.y == 0 && segment.diff.z == 0) {
		assert("線の成分が全て0");
	}

	//6つの平面を構造体に入れる
	Plane pxmin;
	Plane pxmax;
	Plane pymin;
	Plane pymax;
	Plane pzmin;
	Plane pzmax;
	//法線の値を入力
	pxmin.normal = { 1,0,0 };
	pxmax.normal = { 1,0,0 };
	pymin.normal = { 0,1,0 };
	pymax.normal = { 0,1,0 };
	pzmin.normal = { 0,0,1 };
	pzmax.normal = { 0,0,1 };
	//距離の値を入力
	pxmin.distance = Dot(aabb.min, pxmin.normal);
	pymin.distance = Dot(aabb.min, pymin.normal);
	pzmin.distance = Dot(aabb.min, pzmin.normal);
	pxmax.distance = Dot(aabb.max, pxmax.normal);
	pymax.distance = Dot(aabb.max, pymax.normal);
	pzmax.distance = Dot(aabb.max, pzmax.normal);
	//それぞれの平面と線分の衝突点を求める
	if (IsCollision(segment, pxmin)) {
		Vector3 cp = CollisionPoint(segment, pxmin);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(segment, pxmax)) {
		Vector3 cp = CollisionPoint(segment, pxmax);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(segment, pymin)) {
		Vector3 cp = CollisionPoint(segment, pymin);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(segment, pymax)) {
		Vector3 cp = CollisionPoint(segment, pymax);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(segment, pzmin)) {
		Vector3 cp = CollisionPoint(segment, pzmin);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	if (IsCollision(segment, pzmax)) {
		Vector3 cp = CollisionPoint(segment, pzmax);
		if (
			cp.x >= aabb.min.x && cp.x <= aabb.max.x &&
			cp.y >= aabb.min.y && cp.y <= aabb.max.y &&
			cp.z >= aabb.min.z && cp.z <= aabb.max.z
			) {
			return true;
		}
	}
	//線分がaabb内にある場合→始点と終点がそれぞれmin max内
	Vector3 ep = Add(segment.origin, segment.diff);
	if (
		segment.origin.x >= aabb.min.x && segment.origin.x <= aabb.max.x &&
		segment.origin.y >= aabb.min.y && segment.origin.y <= aabb.max.y &&
		segment.origin.z >= aabb.min.z && segment.origin.z <= aabb.max.z &&
		ep.x >= aabb.min.x && ep.x <= aabb.max.x &&
		ep.y >= aabb.min.y && ep.y <= aabb.max.y &&
		ep.z >= aabb.min.z && ep.z <= aabb.max.z
		) {
		return true;
	}

	//これ以上衝突条件は無い
	return false;
}

bool MyMath::IsCollision(const OBB& obb1, const OBB& obb2)
{
	// 各OBBの頂点をローカル座標で定義
	Vector3 obb1Vertex[8] = {
		{ -obb1.size.x, obb1.size.y, -obb1.size.z },
		{ obb1.size.x, obb1.size.y, -obb1.size.z },
		{ -obb1.size.x, -obb1.size.y, -obb1.size.z },
		{ obb1.size.x, -obb1.size.y, -obb1.size.z },
		{ -obb1.size.x, obb1.size.y, obb1.size.z },
		{ obb1.size.x, obb1.size.y, obb1.size.z },
		{ -obb1.size.x, -obb1.size.y, obb1.size.z },
		{ obb1.size.x, -obb1.size.y, obb1.size.z },
	};

	Vector3 obb2Vertex[8] = {
		{ -obb2.size.x, obb2.size.y, -obb2.size.z },
		{ obb2.size.x, obb2.size.y, -obb2.size.z },
		{ -obb2.size.x, -obb2.size.y, -obb2.size.z },
		{ obb2.size.x, -obb2.size.y, -obb2.size.z },
		{ -obb2.size.x, obb2.size.y, obb2.size.z },
		{ obb2.size.x, obb2.size.y, obb2.size.z },
		{ -obb2.size.x, -obb2.size.y, obb2.size.z },
		{ obb2.size.x, -obb2.size.y, obb2.size.z },
	};

	// ワールド行列
	Matrix4x4 obb1WorldMatrix = {
		obb1.orientations[0].x, obb1.orientations[0].y, obb1.orientations[0].z, 0,
		obb1.orientations[1].x, obb1.orientations[1].y, obb1.orientations[1].z, 0,
		obb1.orientations[2].x, obb1.orientations[2].y, obb1.orientations[2].z, 0,
		obb1.center.x, obb1.center.y, obb1.center.z, 1
	};
	Matrix4x4 obb2WorldMatrix = {
		obb2.orientations[0].x, obb2.orientations[0].y, obb2.orientations[0].z, 0,
		obb2.orientations[1].x, obb2.orientations[1].y, obb2.orientations[1].z, 0,
		obb2.orientations[2].x, obb2.orientations[2].y, obb2.orientations[2].z, 0,
		obb2.center.x, obb2.center.y, obb2.center.z, 1
	};

	// 各頂点をワールド座標に変換
	Vector3 obb1WorldVertex[8];
	Vector3 obb2WorldVertex[8];
	for (int i = 0; i < 8; i++) {
		obb1WorldVertex[i] = Transform(obb1Vertex[i], obb1WorldMatrix);
		obb2WorldVertex[i] = Transform(obb2Vertex[i], obb2WorldMatrix);
	}

	// 分離軸の定義
	Vector3 axes[15] = {
		obb1.orientations[0], obb1.orientations[1], obb1.orientations[2],
		obb2.orientations[0], obb2.orientations[1], obb2.orientations[2],
		Cross(obb1.orientations[0], obb2.orientations[0]),
		Cross(obb1.orientations[0], obb2.orientations[1]),
		Cross(obb1.orientations[0], obb2.orientations[2]),
		Cross(obb1.orientations[1], obb2.orientations[0]),
		Cross(obb1.orientations[1], obb2.orientations[1]),
		Cross(obb1.orientations[1], obb2.orientations[2]),
		Cross(obb1.orientations[2], obb2.orientations[0]),
		Cross(obb1.orientations[2], obb2.orientations[1]),
		Cross(obb1.orientations[2], obb2.orientations[2]),
	};

	// 各分離軸について投影範囲を計算し、重なりを確認
	for (const auto& axis : axes) {
		if (axis == Vector3{ 0, 0, 0 }) continue; // ゼロベクトルは無視

		Vector3 normalizedAxis = Normalize(axis);

		auto [min1, max1] = ProjectOntoAxis(obb1WorldVertex, 8, normalizedAxis);
		auto [min2, max2] = ProjectOntoAxis(obb2WorldVertex, 8, normalizedAxis);

		if (max1 < min2 || max2 < min1) {
			return false; // 分離軸が見つかった → 衝突していない
		}
	}

	return true; // すべての軸で重なりあり → 衝突している
}

bool MyMath::IsCollision(const OBB& obb, const Sphere& sphere)
{
	// ワールド行列
	Matrix4x4 obbWorldMatrix = {
		obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0,
		obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0,
		obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0,
		obb.center.x, obb.center.y, obb.center.z, 1
	};
	// ワールド逆行列
	Matrix4x4 obbWorldMatrixInverse = Inverse(obbWorldMatrix);

	Vector3 centerInOBBLocalSpace = Transform(sphere.center, obbWorldMatrixInverse);
	AABB aabbOBBLocal = { .min = { -obb.size.x, -obb.size.y, -obb.size.z }, .max = obb.size };
	Sphere sphereOBBLocal = { centerInOBBLocalSpace, sphere.radius };

	// ローカル空間で衝突判定
	return IsCollision(aabbOBBLocal, sphereOBBLocal);
}

bool MyMath::IsCollision(const OBB& obb, const Line& line)
{
	// ワールド行列
	Matrix4x4 obbWorldMatrix = {
		obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0,
		obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0,
		obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0,
		obb.center.x, obb.center.y, obb.center.z, 1
	};
	// ワールド逆行列
	Matrix4x4 obbWorldMatrixInverse = Inverse(obbWorldMatrix);


	Vector3 centerInOBBLocalLine = Transform(line.origin, obbWorldMatrixInverse);
	AABB aabbOBBLocal = { .min = { -obb.size.x, -obb.size.y, -obb.size.z }, .max = obb.size };
	Line lineOBBLocal = { centerInOBBLocalLine, line.diff };

	// ローカル空間で衝突判定
	return IsCollision(aabbOBBLocal, lineOBBLocal);
}

bool MyMath::IsCollision(const OBB& obb, const Ray& ray)
{
	// ワールド行列
	Matrix4x4 obbWorldMatrix = {
		obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0,
		obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0,
		obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0,
		obb.center.x, obb.center.y, obb.center.z, 1
	};
	// ワールド逆行列
	Matrix4x4 obbWorldMatrixInverse = Inverse(obbWorldMatrix);


	Vector3 centerInOBBLocalRay = Transform(ray.origin, obbWorldMatrixInverse);
	AABB aabbOBBLocal = { .min = { -obb.size.x, -obb.size.y, -obb.size.z }, .max = obb.size };
	Ray rayOBBLocal = { centerInOBBLocalRay, ray.diff };

	// ローカル空間で衝突判定
	return IsCollision(aabbOBBLocal, rayOBBLocal);
}

bool MyMath::IsCollision(const OBB& obb, const Segment& segment)
{
	// ワールド行列
	Matrix4x4 obbWorldMatrix = {
		obb.orientations[0].x, obb.orientations[0].y, obb.orientations[0].z, 0,
		obb.orientations[1].x, obb.orientations[1].y, obb.orientations[1].z, 0,
		obb.orientations[2].x, obb.orientations[2].y, obb.orientations[2].z, 0,
		obb.center.x, obb.center.y, obb.center.z, 1
	};
	// ワールド逆行列
	Matrix4x4 obbWorldMatrixInverse = Inverse(obbWorldMatrix);


	Vector3 centerInOBBLocalSegment = Transform(segment.origin, obbWorldMatrixInverse);
	AABB aabbOBBLocal = { .min = { -obb.size.x, -obb.size.y, -obb.size.z }, .max = obb.size };
	Segment segmentOBBLocal = { centerInOBBLocalSegment, segment.diff };

	// ローカル空間で衝突判定
	return IsCollision(aabbOBBLocal, segmentOBBLocal);
}

void MyMath::DrawSphere(const Sphere& sphere, Vector4 color, LineDrawer* lineDrawer)
{
	float pi = std::numbers::pi_v<float>;
	const uint32_t kSubdivision = 10;//分割数
	const float kLonEvery = 2.0f * pi / kSubdivision;//経度分割1つ分の角度
	const float kLatEvery = pi / kSubdivision;//緯度分割1つ分の角度
	//緯度の方向に分割 -π/2~π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;//現在の経度
		//経度の方向に分割 -π/2~π/2
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;//現在の緯度
			//world座標系でのa,b,cを求める
			Vector3 a, b, c;
			a = {
				sphere.center.x + sphere.radius * cosf(lat) * cosf(lon),
				sphere.center.y + sphere.radius * sinf(lat),
				sphere.center.z + sphere.radius * cosf(lat) * sinf(lon)
			};
			b = {
				sphere.center.x + sphere.radius * cosf(kLatEvery + lat) * cosf(lon),
				sphere.center.y + sphere.radius * sinf(kLatEvery + lat),
				sphere.center.z + sphere.radius * cosf(kLatEvery + lat) * sinf(lon)
			};
			c = {
				sphere.center.x + sphere.radius * cosf(lat) * cosf(kLonEvery + lon),
				sphere.center.y + sphere.radius * sinf(lat),
				sphere.center.z + sphere.radius * cosf(lat) * sinf(kLonEvery + lon)
			};
			//描画
			lineDrawer->CreateLine(a, b, color);
			lineDrawer->CreateLine(a, c, color);
		}
	}
}


///------------------------------------///
///       演算子のオーバーロード
///------------------------------------///

Vector2 operator+(const Vector2& v1, const Vector2& v2)
{
	return MyMath::Add(v1, v2);
}

Vector2 operator-(const Vector2& v1, const Vector2& v2)
{
	return MyMath::Subtract(v1, v2);
}

Vector2 operator*(float s, const Vector2& v)
{
	return MyMath::Multiply(s, v);
}

Vector2 operator*(const Vector2& v, float s)
{
	return s * v;
}

Vector2 operator/(const Vector2& v, float s)
{
	return MyMath::Multiply(1.0f / s, v);
}

Vector3 operator+(const Vector3& v1, const Vector3& v2)
{
	return MyMath::Add(v1, v2);
}

Vector3 operator-(const Vector3& v1, const Vector3& v2)
{
	return MyMath::Subtract(v1, v2);
}

Vector3 operator*(float s, const Vector3& v)
{
	return MyMath::Multiply(s, v);
}

Vector3 operator*(const Vector3& v, float s)
{
	return s * v;
}

Vector3 operator/(const Vector3& v, float s)
{
	return MyMath::Multiply(1.0f / s, v);
}

Vector3 operator*(const Matrix4x4& mat, const Vector3& vec)
{
	// 行列とベクトルの掛け算
	float x = mat.m[0][0] * vec.x + mat.m[0][1] * vec.y + mat.m[0][2] * vec.z + mat.m[0][3];
	float y = mat.m[1][0] * vec.x + mat.m[1][1] * vec.y + mat.m[1][2] * vec.z + mat.m[1][3];
	float z = mat.m[2][0] * vec.x + mat.m[2][1] * vec.y + mat.m[2][2] * vec.z + mat.m[2][3];
	return Vector3(x, y, z);
}

Vector3 operator*(const Quaternion& q, const Vector3& v) {
	// クォータニオンを正規化
	Quaternion normalized_q = q;
	normalized_q.normalize();

	// ベクトルをクォータニオンとして扱う
	Quaternion q_v(0, v.x, v.y, v.z);

	// クォータニオンの掛け算で回転操作: q * v * q^-1
	Quaternion rotated_q = MyMath::Multiply(MyMath::Multiply(normalized_q, q_v), normalized_q.conjugate());

	return Vector3(rotated_q.x, rotated_q.y, rotated_q.z);
}


Matrix4x4 operator+(const Matrix4x4& m1, const Matrix4x4& m2)
{
	return MyMath::Add(m1, m2);
}

Matrix4x4 operator-(const Matrix4x4& m1, const Matrix4x4& m2)
{
	return MyMath::Subtract(m1, m2);
}

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2)
{
	return MyMath::Multiply(m1, m2);
}

Quaternion operator+(const Quaternion& q1, const Quaternion& q2)
{
	return MyMath::Add(q1, q2);
}

Quaternion operator-(const Quaternion& q1, const Quaternion& q2)
{
	return MyMath::Subtract(q1, q2);
}

Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
{
	return MyMath::Multiply(q1, q2);
}

Quaternion operator*(float s, const Quaternion& q)
{
	return MyMath::Multiply(s, q);
}

Quaternion operator*(const Quaternion& q, float s)
{
	return s * q;
}

Vector3 operator-(const Vector3& v)
{
	return { -v.x,-v.y,-v.z };
}

Vector3 operator+(const Vector3& v)
{
	return v;
}

Quaternion operator-(const Quaternion& q)
{
	return { -q.x,-q.y,-q.z,-q.w };
}

Quaternion operator+(const Quaternion& q)
{
	return q;
}
