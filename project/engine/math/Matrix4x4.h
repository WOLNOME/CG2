#pragma once
#include <stdexcept>

/// <summary>
/// 4x4行列
/// </summary>
struct Matrix4x4 final {
	float m[4][4];

	//複合代入演算子のオーバーロード
    Matrix4x4& operator+=(const Matrix4x4& other) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] += other.m[i][j];
        return *this;
    }
    Matrix4x4& operator-=(const Matrix4x4& other) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] -= other.m[i][j];
        return *this;
    }
    Matrix4x4& operator*=(float scalar) {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] *= scalar;
        return *this;
    }
    Matrix4x4& operator/=(float scalar) {
        if (scalar == 0.0f)
            throw std::runtime_error("Division by zero");
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] /= scalar;
        return *this;
    }

};