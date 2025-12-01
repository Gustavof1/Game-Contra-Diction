#pragma once
#include "Actor.h"

class AlienKid : public Actor
{
public:
    explicit AlienKid(class Game* game);

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
    bool mIsRunningAway;
    bool mIsDying;
    float mDeathTimer;
    bool mHasPlayedActiveSound;
};
