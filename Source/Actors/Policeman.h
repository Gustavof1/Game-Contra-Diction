#pragma once
#include "Actor.h"

class Policeman : public Actor
{
public:
    Policeman(class Game* game, float forwardSpeed = 100.0f);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void Kill() override;

private:
    enum class AIState {
        Idle,
        Aggressive
    };

    enum class IdleSubState {
        Stop,
        Walk
    };

    AIState mAIState;
    IdleSubState mIdleSubState;
    float mIdleTimer;
    float mForwardSpeed;
    bool mIsDying;
    float mDyingTimer;
    
    bool mIsAggressive;
    float mShootCooldown;
    float mShootTimer;
    
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    class AnimatorComponent* mAnimatorComponent;
    
    void UpdateIdle(float deltaTime);
    void UpdateAggressive(float deltaTime);
    void Shoot(const Vector2& direction);
};
