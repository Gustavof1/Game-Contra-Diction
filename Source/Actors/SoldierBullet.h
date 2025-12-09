#pragma once
#include "Actor.h"

class SoldierBullet : public Actor
{
public:
    SoldierBullet(class Game* game, const Vector2& direction, Actor* shooter = nullptr);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;

    Actor* GetShooter() const { return mShooter; }

private:
    float mLifeTime;
    Actor* mShooter;
    class AnimatorComponent* mAnimatorComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
};
