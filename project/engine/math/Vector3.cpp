#include "Vector3.h"

float Vector3::Dot(const Vector3& _v) const
{
    return float(
        x * _v.x +
        y * _v.y +
        z * _v.z
        );
}

Vector3 Vector3::Cross(const Vector3& _v) const
{
    return Vector3(
        y * _v.z - z * _v.y,
        z * _v.x - x * _v.z,
        x * _v.y - y * _v.x
    );
}

float Vector3::LengthWithoutRoot() const
{
    return x * x + y * y + z * z;
}

Vector3 Vector3::Normalize() const
{
    float length = this->Length();
    return Vector3(
        x / length,
        y / length,
        z / length
    );
}

float Vector3::Distance(const Vector3& _destination) const
{
    Vector3 distance = _destination - *this;
    return distance.Length();
}

float Vector3::Projection(const Vector3& _a) const
{
    Vector3 axis = _a.Normalize();
    return (*this).Dot(axis);
}

void Vector3::Theta(float& _azimuth, float& _elevation, const Vector3& _origin) const
{
    Vector3 distance = *this - _origin;
    _azimuth = std::atan2(distance.y, distance.x);
    _elevation = std::atan2(distance.z, sqrtf(distance.x * distance.x + distance.y * distance.y));
    return;
}
