#pragma once
#include "Actor.h"

class PolicemanBullet : public Actor
{
public:
    PolicemanBullet(class Game* game, const Vector2& direction);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;

private:
    float mLifeTime;
    class RectComponent* mRectComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
};
