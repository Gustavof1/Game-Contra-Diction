#pragma once
#include "Actor.h"

class Soldier : public Actor
{
public:
    Soldier(class Game* game);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void Kill() override;

private:
    enum class State {
        Idle,
        Attacking
    };

    State mState;
    int mShotsFired;
    float mShotTimer;
    float mAnimationDuration;
    float mCurrentAnimTime;
    
    bool mIsDying;
    float mDyingTimer;

    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    class AnimatorComponent* mAnimatorComponent;

    void Shoot(const Vector2& direction);
};
