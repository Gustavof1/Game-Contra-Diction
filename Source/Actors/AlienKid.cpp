#include "AlienKid.h"
#include "../Game.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "Spaceman.h"
#include "PlayerBullet.h"
#include <cmath>

AlienKid::AlienKid(Game* game)
    : Actor(game)
    , mMoveSpeed(150.0f)
    , mDetectionRadius(400.0f)
    , mIsRunningAway(false)
    , mIsDying(false)
    , mDeathTimer(1.0f) // 48 frames at 12 fps is 4 seconds? No. 48 frames / 12 fps = 4 seconds. That's long.
    , mHasPlayedActiveSound(false)
{
    // 83ms per frame -> 12 FPS.
    // 48 frames -> 4 seconds animation.
    // Let's set death timer to match animation length roughly.

    mAnimatorComponent = new AnimatorComponent(this, 
        "../Assets/Sprites/AlienKid/alien_child_idle_default.png", 
        "../Assets/Sprites/AlienKid/alien_child_idle_default.json", 
        105, 128, 110);
    
    mAnimatorComponent->AppendSpriteSheet(
        "../Assets/Sprites/AlienKid/alien_child_running_default.png",
        "../Assets/Sprites/AlienKid/alien_child_running_default.json");
    
    mAnimatorComponent->AppendSpriteSheet(
        "../Assets/Sprites/AlienKid/alien_child_walk_default.png",
        "../Assets/Sprites/AlienKid/alien_child_walk_default.json");

    mAnimatorComponent->AppendSpriteSheet(
        "../Assets/Sprites/AlienKid/alien_child_death_animation_default.png",
        "../Assets/Sprites/AlienKid/alien_child_death_animation_default.json");

    // Idle: 0-47
    std::vector<int> idleFrames;
    for(int i = 0; i < 48; ++i) idleFrames.push_back(i);
    mAnimatorComponent->AddAnimation("idle", idleFrames);

    // Run: 48-95
    std::vector<int> runFrames;
    for(int i = 48; i < 96; ++i) runFrames.push_back(i);
    mAnimatorComponent->AddAnimation("run", runFrames);

    // Walk: 96-143
    std::vector<int> walkFrames;
    for(int i = 96; i < 144; ++i) walkFrames.push_back(i);
    mAnimatorComponent->AddAnimation("walk", walkFrames);

    // Death: 144-191
    std::vector<int> deathFrames;
    for(int i = 144; i < 192; ++i) deathFrames.push_back(i);
    mAnimatorComponent->AddAnimation("death", deathFrames);

    mAnimatorComponent->SetAnimation("idle");
    mAnimatorComponent->SetAnimFPS(12.0f);
    mAnimatorComponent->SetSize(20.0f, 28.0f);
    mAnimatorComponent->SetUseFrameAspect(true);

    mRigidBodyComponent = new RigidBodyComponent(this);
    mColliderComponent = new AABBColliderComponent(this, 0, -1, 15, 30, ColliderLayer::Enemy);
}

void AlienKid::OnUpdate(float deltaTime)
{
    if (mIsDying) {
        mDeathTimer -= deltaTime;
        if (mDeathTimer <= 0.0f) {
            SetState(ActorState::Destroy);
        }
        return;
    }

    const Spaceman* player = GetGame()->GetPlayer();
    if (!player) return;

    Vector2 playerPos = player->GetPosition();
    float dist = Vector2::Distance(mPosition, playerPos);
    
    bool playerShooting = player->IsShooting();

    Vector2 velocity = Vector2::Zero;

    if (playerShooting) {
        // Run away
        mIsRunningAway = true;
        if (playerPos.x < mPosition.x) {
            velocity.x = mMoveSpeed * 1.5f; // Run faster
        } else {
            velocity.x = -mMoveSpeed * 1.5f;
        }
    } else if (dist < mDetectionRadius) {
        if (!mHasPlayedActiveSound) {
            GetGame()->GetAudio()->PlaySound("Confused.wav");
            GetGame()->AddFloatingText(mPosition, "Tem joguinho no seu celular tio?", 2.0f);
            mHasPlayedActiveSound = true;
        }
        // Approach
        mIsRunningAway = false;
        if (playerPos.x < mPosition.x) {
            velocity.x = -mMoveSpeed;
        } else {
            velocity.x = mMoveSpeed;
        }
    } else {
        mIsRunningAway = false;
    }

    mRigidBodyComponent->SetVelocity(Vector2(velocity.x, mRigidBodyComponent->GetVelocity().y));

    ManageAnimations();
}

void AlienKid::ManageAnimations()
{
    if (mIsDying) {
        mAnimatorComponent->SetAnimation("death");
        return;
    }

    Vector2 vel = mRigidBodyComponent->GetVelocity();
    if (std::abs(vel.x) > 1.0f) {
        if (mIsRunningAway) {
            mAnimatorComponent->SetAnimation("run");
        } else {
            mAnimatorComponent->SetAnimation("walk");
        }
        
        if (vel.x > 0) {
            SetScale(Vector2(1.0f, 1.0f));
        } else {
            SetScale(Vector2(-1.0f, 1.0f));
        }
    } else {
        mAnimatorComponent->SetAnimation("idle");
    }
}

void AlienKid::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::PlayerProjectile) {
        Kill();
        other->GetOwner()->SetState(ActorState::Destroy); // Destroy bullet
    }
}

void AlienKid::Kill()
{
    if (!mIsDying) {
        mIsDying = true;
        GetGame()->OnNPCKilled(this);
        mColliderComponent->SetEnabled(false);
        mRigidBodyComponent->SetEnabled(false);
        mAnimatorComponent->SetAnimation("death");
        mDeathTimer = 4.0f; // Animation duration approx
    }
}
