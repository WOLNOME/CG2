#pragma once
#include <iostream>
#include <cmath>


struct Quaternion {
    float x;
    float y;
    float z;
    float w;

    // コンストラクタ
    Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) : x(x), y(y), z(z), w(w) {}

    // ノルム（大きさ）の計算
    float norm() const {
        return std::sqrt(w * w + x * x + y * y + z * z);
    }

    // 正規化
    void normalize() {
        float n = norm();
        if (n > 0.0001f) { // ほぼゼロの場合は正規化しない
            x /= n;
            y /= n;
            z /= n;
            w /= n;
        }
        else {
            std::cerr << "Warning: Quaternion norm is too close to zero to normalize\n";
        }
    }

    // 共役
    Quaternion conjugate() const {
        return Quaternion(-x, -y, -z, w);
    }

};