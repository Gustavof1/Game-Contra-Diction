#include "FlowerBoss.h"
#include "../Game.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "PlayerBullet.h"
#include "Spaceman.h"
#include "EnemyLaser.h"
#include "Vine.h"
#include "VineWarning.h"
#include "../Random.h"

FlowerBoss::FlowerBoss(Game* game, const std::string& reason)
    : Actor(game)
    , mHP(15)
    , mPhase(1)
    , mState(0)
    , mStateTimer(0.0f)
    , mAttackCooldown(0.0f)
    , mHasAttacked(false)
{
    // Base: Walking
    mAnim = new AnimatorComponent(this, 
        "../Assets/Sprites/FlowerBoss-ContraDiction/a_plant_alien_walking_walking128x128/a_plant_alien_walking_walking128x128.png",
        "../Assets/Sprites/FlowerBoss-ContraDiction/a_plant_alien_walking_walking128x128/a_plant_alien_walking_walking128x128.json",
        128, 128, 95);
    mAnim->SetUseFrameAspect(true);
    
    std::vector<int> walkFrames;
    for(int i=0; i<24; i++) walkFrames.push_back(i); // Assuming 24 frames
    mAnim->AddAnimation("walk", walkFrames);
    mAnim->SetAnimation("walk");

    // Growing
    size_t growStart = mAnim->AppendSpriteSheet(
        "../Assets/Sprites/FlowerBoss-ContraDiction/a_plant_alien_growing_growing128x128/a_plant_alien_growing_growing128x128.png",
        "../Assets/Sprites/FlowerBoss-ContraDiction/a_plant_alien_growing_growing128x128/a_plant_alien_growing_growing128x128.json"
    );
    std::vector<int> growFrames;
    for(int i=0; i<24; i++) growFrames.push_back(growStart + i);
    mAnim->AddAnimation("grow", growFrames);

    // Punching
    size_t punchStart = mAnim->AppendSpriteSheet(
        "../Assets/Sprites/FlowerBoss-ContraDiction/a_plant_alien_punching_punching-1-128x128/a_plant_alien_punching_punching-1-128x128.png",
        "../Assets/Sprites/FlowerBoss-ContraDiction/a_plant_alien_punching_punching-1-128x128/a_plant_alien_punching_punching-1-128x128.json"
    );
    std::vector<int> punchFrames;
    for(int i=0; i<24; i++) punchFrames.push_back(punchStart + i);
    mAnim->AddAnimation("punch", punchFrames);

    // Flying
    size_t flyStart = mAnim->AppendSpriteSheet(
        "../Assets/Sprites/FlowerBoss-ContraDiction/a_plant_alien_flying_flying128x128/a_plant_alien_flying_flying128x128.png",
        "../Assets/Sprites/FlowerBoss-ContraDiction/a_plant_alien_flying_flying128x128/a_plant_alien_flying_flying128x128.json"
    );
    std::vector<int> flyFrames;
    for(int i=0; i<24; i++) flyFrames.push_back(flyStart + i);
    mAnim->AddAnimation("fly", flyFrames);

    mRigidBody = new RigidBodyComponent(this);
    mRigidBody->SetApplyGravity(true);
    mCollider = new AABBColliderComponent(this, 20, 20, 80, 100, ColliderLayer::Enemy);

    // Text
    if (reason == "kill") {
        GetGame()->AddFloatingText(GetPosition(), "Voce nao devia ter feito isso, agora ira conhecer a minha forca", 4.0f, this);
    } else if (reason == "ignore") {
        GetGame()->AddFloatingText(GetPosition(), "Como ousa ignorar uma flor tao bonita como eu?", 4.0f, this);
    }

    ChangeState(0); // Start growing
}

void FlowerBoss::OnUpdate(float deltaTime)
{
    mStateTimer += deltaTime;
    mAttackCooldown -= deltaTime;

    auto player = GetGame()->GetPlayer();
    if (!player) return;

    Vector2 diff = player->GetPosition() - GetPosition();
    float dist = diff.Length();
    int dir = (diff.x > 0) ? 1 : -1;
    
    if (mPhase == 1) {
        SetScale(Vector2(dir * 1.0f, 1.0f));
        
        switch (mState)
        {
        case 0: // Spawn/Grow
            if (mStateTimer > 1.0f) {
                ChangeState(1);
            }
            break;
        case 1: // Idle/Decide
            mRigidBody->SetVelocity(Vector2::Zero);
            if (mStateTimer > 0.5f) { // Wait a bit before deciding
                float rand = Random::GetFloat();
                if (dist < 100.0f) {
                    // Close range: Punch (70%), Fly (10%), Crouch (20%)
                    if (rand < 0.7f) ChangeState(2);
                    else if (rand < 0.8f) ChangeState(3);
                    else ChangeState(4);
                } else {
                    // Far range: Walk (40%), Fly (30%), Crouch (30%)
                    if (rand < 0.4f) ChangeState(5); // Walk
                    else if (rand < 0.7f) ChangeState(3); // Fly
                    else ChangeState(4); // Crouch
                }
            }
            break;
        case 2: // Attack (Punch)
            if (mStateTimer > 0.5f) {
                ChangeState(1);
            }
            break;
        case 3: // Fly
            mRigidBody->SetVelocity(Vector2(dir * 120.0f, -50.0f));
            
            if (mAttackCooldown <= 0.0f) {
                 mAttackCooldown = 1.5f;
                 auto* bullet = new EnemyLaser(GetGame(), this);
                 bullet->SetPosition(GetPosition());
                 Vector2 vel = diff;
                 vel.Normalize();
                 bullet->SetVelocity(vel * 400.0f);
            }

            // Fly for a bit then land
            if (mStateTimer > 3.0f) {
                ChangeState(1);
            }
            break;
        case 4: // Crouch Attack
            mRigidBody->SetVelocity(Vector2::Zero);
            if (mStateTimer > 0.1f && !mHasAttacked) {
                mHasAttacked = true;
                // Spawn Warnings
                if (player) {
                    for (int i = 0; i < 3; i++) {
                        auto* warn = new VineWarning(GetGame());
                        Vector2 spawnPos = player->GetPosition();
                        spawnPos.x += (i - 1) * 40.0f; 
                        spawnPos.y += 64.0f; 
                        warn->SetPosition(spawnPos);
                    }
                }
            }
            // Spawn Vines after 1s
            // Handled by VineWarning death
            
            if (mStateTimer > 2.0f) {
                ChangeState(1);
            }
            break;
        case 5: // Walk/Chase
            mRigidBody->SetVelocity(Vector2(dir * 80.0f, mRigidBody->GetVelocity().y));
            if (dist < 80.0f || mStateTimer > 2.0f) {
                ChangeState(1);
            }
            break;
        }
    } else { // Phase 2
        // Static, shoot
        mRigidBody->SetVelocity(Vector2::Zero);
        SetScale(Vector2(dir * 1.0f, 1.0f)); // Face player
        
        if (mState == 0) { // Growing transition
             if (mStateTimer > 1.0f) {
                 ChangeState(1); // Idle/Shoot
             }
        } else {
             // Shoot logic
             if (mAttackCooldown <= 0.0f) {
                 mAttackCooldown = 2.0f;
                 // Shoot
                 auto* bullet = new EnemyLaser(GetGame(), this);
                 bullet->SetPosition(GetPosition());
                 Vector2 vel = diff;
                 vel.Normalize();
                 bullet->SetVelocity(vel * 400.0f);
             }
        }
    }
}

void FlowerBoss::ChangeState(int state)
{
    mState = state;
    mStateTimer = 0.0f;
    
    if (state == 3) { // Fly
        mRigidBody->SetApplyGravity(false);
    } else {
        mRigidBody->SetApplyGravity(true);
    }
    
    switch (state)
    {
    case 0: mAnim->SetAnimation("grow"); break;
    case 1: mAnim->SetAnimation("walk"); break;
    case 2: mAnim->SetAnimation("punch"); break;
    case 3: mAnim->SetAnimation("fly"); break;
    case 4: mAnim->SetAnimation("grow"); mHasAttacked = false; break;
    case 5: mAnim->SetAnimation("walk"); break;
    }
}

void FlowerBoss::Kill()
{
    mHP--;
    if (mHP <= 0) {
        if (mPhase == 1) {
            mPhase = 2;
            mHP = 30;
            ChangeState(0); // Grow again
        } else {
            SetState(ActorState::Destroy);
        }
    }
}

void FlowerBoss::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::PlayerProjectile) {
        Kill();
        other->GetOwner()->SetState(ActorState::Destroy);
    } else if (other->GetLayer() == ColliderLayer::Player) {
        other->GetOwner()->Kill();
    }
}
