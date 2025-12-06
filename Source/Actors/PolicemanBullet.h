#pragma once
#include "Actor.h"

class PolicemanBullet : public Actor
{
public:
    PolicemanBullet(class Game* game, const Vector2& direction, Actor* shooter = nullptr);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;

    Actor* GetShooter() const { return mShooter; }

private:
    float mLifeTime;
    Actor* mShooter;
    class RectComponent* mRectComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
};
