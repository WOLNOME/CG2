#pragma once
#include <stdexcept>

/// <summary>
/// 3x3行列
/// </summary>
struct Matrix3x3 final {
	float m[3][3];

    //複合代入演算子のオーバーロード
    Matrix3x3& operator+=(const Matrix3x3& other) {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] += other.m[i][j];
        return *this;
    }
    Matrix3x3& operator-=(const Matrix3x3& other) {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] -= other.m[i][j];
        return *this;
    }
    Matrix3x3& operator*=(float scalar) {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] *= scalar;
        return *this;
    }
    Matrix3x3& operator/=(float scalar) {
        if (scalar == 0.0f)
            throw std::runtime_error("Division by zero");
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] /= scalar;
        return *this;
    }
};