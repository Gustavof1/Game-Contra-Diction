//
// Created by Lucas N. Ferreira on 18/09/25.
//

#include "../Game.h"
#include "ParticleSystemComponent.h"
#include "Physics/AABBColliderComponent.h"
#include "Physics/RigidBodyComponent.h"
#include "Drawing/AnimatorComponent.h"

Particle::Particle(class Game* game, int width, int height, const std::string& texturePath, const std::string& jsonPath, ColliderLayer layer, bool useGravity, float colliderScale, const Vector2& drawOffset)
    : Actor(game)
    , mAnimator(nullptr)
    , mRigidBodyComponent(nullptr)
    , mColliderComponent(nullptr)
    , mIsDead(true)
    , mLifeTime(1.0f)
    , mIsExploding(false)
{
    mAnimator = new AnimatorComponent(this, texturePath, jsonPath, width, height);
    mAnimator->SetOffset(drawOffset);
    mRigidBodyComponent = new RigidBodyComponent(this, 1.0f, 0.0f, useGravity);
    mColliderComponent = new AABBColliderComponent(this, 0, 0, static_cast<int>(width * colliderScale), static_cast<int>(height * colliderScale), layer, true);

    SetState(ActorState::Paused);
    mAnimator->SetVisible(false);
}

void Particle::Kill()
{
    mIsDead = true;
    mIsExploding = false;
    SetState(ActorState::Paused);
    mAnimator->SetVisible(false);
    mColliderComponent->SetEnabled(false);

    // Reset velocity
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
}

void Particle::Awake(const Vector2 &position, float rotation, float lifetime)
{
    mLifeTime = lifetime;
    mIsExploding = false;

    mIsDead = false;
    SetState(ActorState::Active);
    mAnimator->SetVisible(true);
    mColliderComponent->SetEnabled(true);

    SetPosition(position);
    SetRotation(rotation);

    mAnimator->SetAnimation("fly");
    mAnimator->SetLooping(true);
}

void Particle::OnUpdate(float deltaTime)
{
    if (mIsExploding)
    {
        if (mAnimator->IsFinished())
        {
            Kill();
        }
        return;
    }

    mLifeTime -= deltaTime;
    if (mLifeTime <= 0)
    {
        mIsExploding = true;
        mAnimator->SetAnimation("explode");
        mAnimator->SetLooping(false);
        mRigidBodyComponent->SetVelocity(Vector2::Zero);
        mColliderComponent->SetEnabled(false);
        return;
    }
}

void Particle::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (mIsExploding) return;

    if (other->IsTrigger())
    {
        return;
    }

    if (mOnCollision)
    {
        mOnCollision(other);
    }

    mIsExploding = true;
    mAnimator->SetAnimation("explode");
    mAnimator->SetLooping(false);
    mRigidBodyComponent->SetVelocity(Vector2::Zero);
    mColliderComponent->SetEnabled(false);
}

void Particle::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    OnHorizontalCollision(minOverlap, other);
}

ParticleSystemComponent::ParticleSystemComponent(class Actor* owner, int particleW, int particleH, 
                                                 const std::string& texturePath, const std::string& jsonPath,
                                                 int poolSize, int updateOrder, ColliderLayer collisionLayer,
                                                 bool useGravity, float colliderScale, const Vector2& drawOffset)
    : Component(owner, updateOrder)
{
    // Create a pool of particles
    for (int i = 0; i < poolSize; i++)
    {
        auto* p = new Particle(owner->GetGame(), particleW, particleH, texturePath, jsonPath, collisionLayer, useGravity, colliderScale, drawOffset);
        
        p->GetAnimator()->AddAnimation("fly", {0});
        p->GetAnimator()->AddAnimation("explode", {1, 2, 3});
        p->GetAnimator()->SetAnimation("fly");

        mParticles.push_back(p);
    }
}

void ParticleSystemComponent::SetCollisionCallback(std::function<void(class AABBColliderComponent*)> callback)
{
    for (auto p : mParticles)
    {
        p->SetOnCollision(callback);
    }
}

void ParticleSystemComponent::EmitParticle(float lifetime, float speed, const Vector2& direction, const Vector2& offsetPosition)
{
    for (auto p : mParticles)
    {
        if (p->IsDead())
        {
            // Wake up the particle
            Vector2 spawnPos = mOwner->GetPosition() + offsetPosition * mOwner->GetScale().x;
            p->Awake(spawnPos, mOwner->GetRotation(), lifetime);

            // Forward velocity
            p->GetComponent<RigidBodyComponent>()->SetVelocity(direction * speed);

            // Break inner loop to emit only one particle per iteration
            break;
        }
    }
}