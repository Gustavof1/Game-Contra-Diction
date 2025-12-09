#pragma once
#include "Actor.h"

class FlowerBoss : public Actor
{
public:
    FlowerBoss(class Game* game, const std::string& reason);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void Kill() override;

private:
    void ChangeState(int state);
    
    int mHP;
    int mPhase;
    int mState; // 0: Spawn, 1: Chase, 2: Attack, 3: Fly, 4: CrouchAttack
    float mStateTimer;
    float mAttackCooldown;
    bool mHasAttacked;
    int mAttackCount;
    
    class AnimatorComponent* mAnim;
    class RigidBodyComponent* mRigidBody;
    class AABBColliderComponent* mCollider;
};
