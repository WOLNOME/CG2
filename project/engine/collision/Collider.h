#pragma once
#include "Vector3.h"
#include <cstdint>
class Collider {

public:
	virtual ~Collider() = default;
	float GetRadius() { return radius_; }
	virtual void OnCollision();
	virtual Vector3 GetWorldPosition() = 0;
	uint32_t GetCollisionAttribute() { return collisionAttribute_; }
	void SetCollisionAttribute(const uint32_t collisionAttribute) { collisionAttribute_ = collisionAttribute; }

private:
	uint32_t collisionAttribute_ = 0xffffffff;

protected:
	float radius_;
};
