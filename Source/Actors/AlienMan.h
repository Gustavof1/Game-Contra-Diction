#pragma once
#include "Actor.h"

class AlienMan : public Actor
{
public:
    explicit AlienMan(class Game* game);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void Kill() override;

private:
    void ManageAnimations();

    class RigidBodyComponent* mRigidBodyComponent;
    class AnimatorComponent* mAnimatorComponent;
    class AABBColliderComponent* mColliderComponent;

    float mMoveSpeed;
    float mDetectionRadius;
    float mMaintainDistance;
    bool mIsRunningAway;
    bool mIsDying;
    float mDeathTimer;
    bool mHasPlayedActiveSound;
};
