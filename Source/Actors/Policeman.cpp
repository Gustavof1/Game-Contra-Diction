#include "Policeman.h"
#include "PolicemanBullet.h"
#include "Spaceman.h"
#include "../Game.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include <cmath>

Policeman::Policeman(Game* game, float forwardSpeed)
    : Actor(game)
    , mForwardSpeed(forwardSpeed)
    , mIsDying(false)
    , mDyingTimer(0.5f)
    , mAIState(AIState::Idle)
    , mIdleSubState(IdleSubState::Stop)
    , mIdleTimer(1.0f)
    , mIsAggressive(false)
    , mShootCooldown(1.5f)
    , mShootTimer(0.0f)
{
    // Load animations
    mAnimatorComponent = new AnimatorComponent(this, 
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_idle_idle.png", 
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_idle_idle.json", 
        127, 128, 110);
    
    std::vector<int> idleFrames;
    size_t count = mAnimatorComponent->GetFrameCount();
    for(size_t i=0; i<count; ++i) idleFrames.push_back(i);
    mAnimatorComponent->AddAnimation("idle", idleFrames);
    
    size_t start = mAnimatorComponent->GetFrameCount();
    mAnimatorComponent->AppendSpriteSheet(
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_idle_idlewalk.png",
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_idle_idlewalk.json");
    size_t end = mAnimatorComponent->GetFrameCount();
    std::vector<int> idleWalkFrames;
    for(size_t i=start; i<end; ++i) idleWalkFrames.push_back(i);
    mAnimatorComponent->AddAnimation("idlewalk", idleWalkFrames);
    
    start = end;
    mAnimatorComponent->AppendSpriteSheet(
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_walk_walk.png",
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_walk_walk.json");
    end = mAnimatorComponent->GetFrameCount();
    std::vector<int> walkFrames;
    for(size_t i=start; i<end; ++i) walkFrames.push_back(i);
    mAnimatorComponent->AddAnimation("walk", walkFrames);
    
    start = end;
    mAnimatorComponent->AppendSpriteSheet(
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_attack_shooting.png",
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_attack_shooting.json");
    end = mAnimatorComponent->GetFrameCount();
    std::vector<int> shootFrames;
    for(size_t i=start; i<end; ++i) shootFrames.push_back(i);
    mAnimatorComponent->AddAnimation("shoot", shootFrames);
    
    start = end;
    mAnimatorComponent->AppendSpriteSheet(
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_attack_punch.png",
        "../Assets/Sprites/Policeman-ContraDiction/enemy_generic_alien_soldier_attack_punch.json");
    end = mAnimatorComponent->GetFrameCount();
    std::vector<int> punchFrames;
    for(size_t i=start; i<end; ++i) punchFrames.push_back(i);
    mAnimatorComponent->AddAnimation("punch", punchFrames);
    
    mAnimatorComponent->SetAnimation("idle");
    mAnimatorComponent->SetAnimFPS(10.0f);
    mAnimatorComponent->SetSize(63.5f, 64.0f);
    mAnimatorComponent->SetUseFrameAspect(true);

    mRigidBodyComponent = new RigidBodyComponent(this);
    mColliderComponent = new AABBColliderComponent(this, 0, 2, 30, 60, ColliderLayer::Enemy);
}

void Policeman::OnUpdate(float deltaTime)
{
    if (mIsDying) {
        mDyingTimer -= deltaTime;
        if (mDyingTimer <= 0.0f) {
            SetState(ActorState::Destroy);
        }
        return;
    }
    
    const Spaceman* player = GetGame()->GetPlayer();
    if (!player) return;
    
    float dist = Vector2::Distance(mPosition, player->GetPosition());
    
    if (!mIsAggressive) {
        if (dist < 400.0f) { // Detection distance
            mIsAggressive = true;
            mAIState = AIState::Aggressive;
            GetGame()->GetAudio()->PlaySound("Confused.wav");
        } else {
            UpdateIdle(deltaTime);
        }
    } else {
        UpdateAggressive(deltaTime);
    }
    
    if (mPosition.y > Game::WINDOW_HEIGHT + Game::TILE_SIZE) {
        SetState(ActorState::Destroy);
    }
}

void Policeman::UpdateIdle(float deltaTime)
{
    mIdleTimer -= deltaTime;
    if (mIdleTimer <= 0.0f) {
        if (mIdleSubState == IdleSubState::Stop) {
            mIdleSubState = IdleSubState::Walk;
            mIdleTimer = 2.0f;
            mAnimatorComponent->SetAnimation("idlewalk");
            mRigidBodyComponent->SetVelocity(Vector2(50.0f, 0.0f)); 
            mScale.x = -1.0f;
        } else {
            mIdleSubState = IdleSubState::Stop;
            mIdleTimer = 1.0f;
            mAnimatorComponent->SetAnimation("idle");
            mRigidBodyComponent->SetVelocity(Vector2(0.0f, 0.0f));
        }
    }
}

void Policeman::UpdateAggressive(float deltaTime)
{
    const Spaceman* player = GetGame()->GetPlayer();
    if (!player) return;
    
    Vector2 playerPos = player->GetPosition();
    float dist = Vector2::Distance(mPosition, playerPos);
    Vector2 dir = playerPos - mPosition;
    dir.Normalize();
    
    bool isAttacking = (mAnimatorComponent->GetAnimName() == "punch" || mAnimatorComponent->GetAnimName() == "shoot");

    if (dir.x > 0) mScale.x = isAttacking ? 1.0f : -1.0f;
    else mScale.x = isAttacking ? -1.0f : 1.0f;
    
    mShootTimer -= deltaTime;
    
    if (dist <= 50.0f) { // Punch distance
        mAnimatorComponent->SetAnimation("punch");
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
    } else if (dist <= 300.0f && player->IsShooting()) { // Shoot distance
        mAnimatorComponent->SetAnimation("shoot");
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
        if (mShootTimer <= 0.0f) {
            Shoot(dir);
            mShootTimer = mShootCooldown;
        }
    } else { // Chase
        mAnimatorComponent->SetAnimation("walk");
        mRigidBodyComponent->SetVelocity(Vector2(dir.x * mForwardSpeed, mRigidBodyComponent->GetVelocity().y));
    }
}

void Policeman::Shoot(const Vector2& direction)
{
    GetGame()->GetAudio()->PlaySound("Shoot.wav");
    auto* bullet = new PolicemanBullet(GetGame(), direction, this);
    bullet->SetPosition(mPosition + direction * 40.0f);
}

void Policeman::Kill()
{
    if (mIsDying) return;
    mIsDying = true;
    GetGame()->OnNPCKilled(this);
    mColliderComponent->SetEnabled(false);
    mRigidBodyComponent->SetVelocity(Vector2(0.0f, -350.0f));
}

void Policeman::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    // Standard collision
}

void Policeman::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    // Standard collision
}
