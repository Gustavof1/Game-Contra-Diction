#include "Soldier.h"
#include "SoldierBullet.h"
#include "Spaceman.h"
#include "../Game.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

Soldier::Soldier(Game* game)
    : Actor(game)
    , mState(State::Idle)
    , mShotsFired(0)
    , mShotTimer(0.0f)
    , mCurrentAnimTime(0.0f)
    , mIsDying(false)
    , mDyingTimer(0.5f)
{
    mAnimatorComponent = new AnimatorComponent(this, 
        "../Assets/Sprites/Soldier/generic_alien_soldier_walk_default.png", 
        "../Assets/Sprites/Soldier/generic_alien_soldier_walk_default.json",
        126, 128, 110);
    
    // Walk animation
    std::vector<int> walkFrames;
    size_t count = mAnimatorComponent->GetFrameCount();
    for(size_t i=0; i<count; ++i) walkFrames.push_back(i);
    mAnimatorComponent->AddAnimation("walk", walkFrames);
    
    // Attack animation
    size_t start = mAnimatorComponent->GetFrameCount();
    mAnimatorComponent->AppendSpriteSheet(
        "../Assets/Sprites/Soldier/generic_alien_soldier_attack_default.png",
        "../Assets/Sprites/Soldier/generic_alien_soldier_attack_default.json");
    size_t end = mAnimatorComponent->GetFrameCount();
    std::vector<int> attackFrames;
    for(size_t i=start; i<end; ++i) attackFrames.push_back(i);
    mAnimatorComponent->AddAnimation("attack", attackFrames);

    mAnimatorComponent->SetAnimation("walk");
    mAnimatorComponent->SetAnimFPS(10.0f);
    mAnimatorComponent->SetSize(63.0f, 64.0f); // Adjust size if needed
    mAnimatorComponent->SetUseFrameAspect(true);
    
    // Calculate duration of attack animation
    // Frame count for attack is (end - start)
    float frameCount = static_cast<float>(end - start);
    mAnimationDuration = frameCount / 10.0f; // 10 FPS

    mRigidBodyComponent = new RigidBodyComponent(this);
    mColliderComponent = new AABBColliderComponent(this, 0, -8, 40, 80, ColliderLayer::Enemy);
}

void Soldier::OnUpdate(float deltaTime)
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
    Vector2 dir = player->GetPosition() - mPosition;
    dir.Normalize();

    // Face player
    bool isAttacking = (mState == State::Attacking);
    if (dir.x > 0) mScale.x = isAttacking ? 1.0f : -1.0f;
    else mScale.x = isAttacking ? -1.0f : 1.0f;

    if (mState == State::Idle) {
        mAnimatorComponent->SetAnimation("walk");
        mAnimatorComponent->SetAnimFPS(0.0f); // Pause for idle
        
        if (dist < 400.0f) {
            mState = State::Attacking;
            GetGame()->GetAudio()->PlaySound("Confused.wav");
            GetGame()->AddFloatingText(mPosition, "Alien encontrado, exterminar", 2.0f, this);
            mShotsFired = 0;
            mCurrentAnimTime = 0.0f;
            mShotTimer = 0.0f;
            mAnimatorComponent->SetAnimation("attack");
            mAnimatorComponent->SetAnimFPS(10.0f);
        }
    }
    else if (mState == State::Attacking) {
        mCurrentAnimTime += deltaTime;
        
        // Shooting logic
        if (mShotsFired < 3) {
            mShotTimer -= deltaTime;
            if (mShotTimer <= 0.0f) {
                Shoot(dir);
                mShotsFired++;
                // Distribute shots evenly across animation
                mShotTimer = mAnimationDuration / 3.5f; 
            }
        }

        if (mCurrentAnimTime >= mAnimationDuration) {
            // Animation finished
            if (dist > 450.0f) {
                mState = State::Idle;
            } else {
                // Restart attack
                mShotsFired = 0;
                mCurrentAnimTime = 0.0f;
                mShotTimer = 0.0f;
                mAnimatorComponent->SetAnimation("attack"); // Reset frame
            }
        }
    }
    
    if (mPosition.y > Game::WINDOW_HEIGHT + Game::TILE_SIZE) {
        SetState(ActorState::Destroy);
    }
}

void Soldier::Shoot(const Vector2& direction)
{
    GetGame()->GetAudio()->PlaySound("Continuousshooting.wav");
    auto* bullet = new SoldierBullet(GetGame(), direction, this);
    bullet->SetPosition(mPosition + direction * 40.0f);
}

void Soldier::Kill()
{
    if (mIsDying) return;
    mIsDying = true;
    GetGame()->OnNPCKilled(this);
    mColliderComponent->SetEnabled(false);
    mRigidBodyComponent->SetVelocity(Vector2(0.0f, -350.0f));
}

void Soldier::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
}

void Soldier::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
}
