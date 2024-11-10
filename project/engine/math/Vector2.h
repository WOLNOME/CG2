#pragma once
#include <stdexcept>
#include <cmath>

/// <summary>
/// 2次元ベクトル
/// </summary>
struct Vector2 final {
	float x;
	float y;
	//初期化リスト
	Vector2(float x = 0, float y = 0) : x(x), y(y) {}
	//複合代入演算子のオーバーロード
	Vector2& operator+=(const Vector2& other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	Vector2& operator-=(const Vector2& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}
	Vector2& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		return *this;
	}
	Vector2& operator/=(float scalar) {
		if (scalar == 0.0f)
			throw std::runtime_error("Division by zero");
		x /= scalar;
		y /= scalar;
		return *this;
	}
	bool operator==(const Vector2& other) const {
		return x == other.x && y == other.y;
	}

	bool operator!=(const Vector2& other) const {
		return !(*this == other);
	}
	//ベクトルの長さを取得
	float Length() const {
		return std::sqrt(x * x + y * y);
	}
	//ベクトルを正規化した値を出力
	Vector2 OutputNormalized() const {
		float len = Length();
		if (len == 0.0f) return Vector2(0.0f, 0.0f);
		return Vector2(x / len, y / len);
	}
	//自分自身を正規化
	Vector2& Normalize() {
		float len = Length();
		if (len == 0.0f) {
			x = y = 0.0f;
			return *this;
		}
		x /= len;
		y /= len;
		return *this;
	}
};
