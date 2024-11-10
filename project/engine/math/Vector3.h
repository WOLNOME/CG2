#pragma once
#include <stdexcept>
#include <cmath>

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 final {
	float x;
	float y;
	float z;
	//初期化リスト
	Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
	//複合代入演算子のオーバーロード
	Vector3& operator+=(const Vector3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	Vector3& operator-=(const Vector3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	Vector3& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
	Vector3& operator/=(float scalar) {
		if (scalar == 0.0f)
			throw std::runtime_error("Division by zero");
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}
	bool operator==(const Vector3& other) const {
		return x == other.x && y == other.y && z == other.z;
	}

	bool operator!=(const Vector3& other) const {
		return !(*this == other);
	}
	//ベクトルの長さを取得
	float Length() const {
		return std::sqrt(x * x + y * y + z * z);
	}
	//ベクトルを正規化した値を出力
	Vector3 Normalized() const {
		float len = Length();
		if (len == 0.0f) return Vector3(0.0f, 0.0f, 0.0f);
		return Vector3(x / len, y / len, z / len);
	}
	//自分自身を正規化
	Vector3& Normalize() {
		float len = Length();
		if (len == 0.0f) {
			x = y = z = 0.0f;
			return *this;
		}
		x /= len;
		y /= len;
		z /= len;
		return *this;
	}
};