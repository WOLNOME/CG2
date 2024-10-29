#pragma once
#include <vector>
#include <cassert>
#include <string>
#include <cmath>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
//列挙型
enum LightKind {
	HalfLambert,
	Lambert,
	NoneLighting
};

//構造体
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

struct Sphere {
	Vector3 center;
	float radius;
};



//関数
Vector3 Add(const Vector3& v1, const Vector3& v2);

Vector3 Subtract(const Vector3& v1, const Vector3& v2);

Vector3 Multiply(float s, const Vector3& v);

Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

Matrix4x4 Inverse(const Matrix4x4& m);

Matrix4x4 Transpose(const Matrix4x4& m);

Matrix4x4 MakeIdentity4x4();

Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

Matrix4x4 MakeScaleMatrix(const Vector3& scale);

Matrix4x4 MakeRotateXMatrix(float radian);

Matrix4x4 MakeRotateYMatrix(float radian);

Matrix4x4 MakeRotateZMatrix(float radian);

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

float Cot(float rad);

Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

Matrix4x4 makeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

Vector3 Cross(const Vector3& a, const Vector3& b);

float Length(const Vector3& v);

Vector3 Normalize(const Vector3& v);
