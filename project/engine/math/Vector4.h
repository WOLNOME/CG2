#pragma once
#include <stdexcept>
#include <cmath>

/// <summary>
/// 4次元ベクトル
/// </summary>
struct Vector4 final {
	float x;
	float y;
	float z;
	float w;
	//初期化リスト
	Vector4(float x = 0, float y = 0, float z = 0, float w = 0) : x(x), y(y), z(z), w(w) {}
	//複合代入演算子のオーバーロード
	Vector4& operator+=(const Vector4& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}
	Vector4& operator-=(const Vector4& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}
	Vector4& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return *this;
	}
	Vector4& operator/=(float scalar) {
		if (scalar == 0.0f)
			throw std::runtime_error("Division by zero");
		x /= scalar;
		y /= scalar;
		z /= scalar;
		w /= scalar;
		return *this;
	}
	bool operator==(const Vector4& other) const {
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	bool operator!=(const Vector4& other) const {
		return !(*this == other);
	}
	// ベクトルの長さを計算
	float Length() const {
		return std::sqrt(x * x + y * y + z * z + w * w);
	}
	// ベクトルの正規化を返す
	Vector4 Normalized() const {
		float len = Length();
		if (len == 0.0f)
			throw std::runtime_error("Cannot normalize a zero-length vector");
		return Vector4(x / len, y / len, z / len, w / len);
	}
	// 自分自身を正規化する
	Vector4& Normalize() {
		float len = Length();
		if (len == 0.0f)
			throw std::runtime_error("Cannot normalize a zero-length vector");
		x /= len;
		y /= len;
		z /= len;
		w /= len;
		return *this;
	}
};
