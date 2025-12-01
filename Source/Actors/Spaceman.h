#pragma once

#include "Actor.h"
#include "../Inventory.h"

enum class PlayerPosture
{
    Standing,   // Em pé
    Crouching,  // Agachado
    Prone       // Deitado
};

class Spaceman : public Actor
{
public:
    explicit Spaceman(class Game* game);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;

    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;

    void Kill() override;

    bool IsShooting() const { return mIsShooting || mIsFiringLaser; }
    bool IsDancing() const { return mIsDancing; }

    void PowerUp();
    void PowerDown();

    // Inventory System
    void EquipHead(ItemType item);
    void EquipHand(ItemType item);
    ItemType GetHeadItem() const { return mHeadItem; }
    ItemType GetHandItem() const { return mHandItem; }

private:
    void ManageAnimations();
    void TryShoot();
    void StopShoot();
    void PerformJump();
    void SetPosture(PlayerPosture posture);

    class SpacemanArm* mArm;
    class Laser* mLaser;
    class ParticleSystemComponent* mBulletParticleSystem;
    Vector2 mShootDirection;

    // Inventory State
    ItemType mHeadItem;
    ItemType mHandItem;

    float mMoveForce;
    float mJumpImpulse;
    float mShootCooldown;
    float mShootTimer;
    float mShootAnimTimer;

    bool mSPressedLastFrame; 
    PlayerPosture mPosture;

    int mJumpCount;     
    bool mWPressedLastFrame;
    bool mTabPressedLastFrame;

    bool mIsRunning;
    bool mIsDead;
    bool mIsShooting;
    bool mIsFiringLaser;
    bool mIsCrouchingHeld;
    bool mIsDancing;
    float mDeathTimer;

    class RigidBodyComponent* mRigidBodyComponent;
    class AnimatorComponent* mAnimatorComponent;
    class AnimatorComponent* mPeaceAnimator;
    class AABBColliderComponent* mColliderComponent;
};
