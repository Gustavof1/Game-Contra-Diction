//
// Created by Lucas N. Ferreira on 30/09/23.
//

#include "Goomba.h"
#include "Mushroom.h"
#include "../Game.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

Goomba::Goomba(Game* game, float forwardSpeed, float deathTime)
        : Actor(game)
        , mDyingTimer(deathTime)
        , mIsDying(false)
        , mForwardSpeed(forwardSpeed)
        , mStomped(false)
{
    // Create animator component
    auto* ac = new AnimatorComponent(this, "../Assets/Sprites/Goomba/Goomba.png", "../Assets/Sprites/Goomba/Goomba.json", Game::TILE_SIZE, Game::TILE_SIZE, 99);
    ac->AddAnimation("dead", {0});
    ac->AddAnimation("walk", {1, 2});
    ac->SetAnimation("walk");
    ac->SetAnimFPS(10.0f);

    // Create rigid body component
    new RigidBodyComponent(this);

    // Create AABB collider component
    new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE, Game::TILE_SIZE, ColliderLayer::Enemy);
}

void Goomba::Kill()
{
    if (mIsDying) {
        return;
    }
    
    mIsDying = true;

    // Disable AABB collider
    auto* aabb = GetComponent<AABBColliderComponent>();
    if (aabb) {
        aabb->SetEnabled(false);
    }

    if (mStomped) {
    
        // Change animation
        auto* ac = GetComponent<AnimatorComponent>();
        if (ac) {
            ac->SetAnimation("dead");
        }

        // Disable physics 
        auto* rbc = GetComponent<RigidBodyComponent>();
        if (rbc) {
            rbc->SetEnabled(false);
        }
    }
    else {

        auto* rbc = GetComponent<RigidBodyComponent>();
        if (rbc) {
            float horizontalVelocity = (rand() % 2 == 0) ? 150.0f : -150.0f;
            rbc->SetVelocity(Vector2(horizontalVelocity, -350.0f));
        }

        mScale.y = -1.0f;
    }
    

    
}

void Goomba::OnUpdate(float deltaTime)
{
    if (mIsDying) {
        if (mStomped) { // countdown to destroy
            mDyingTimer -= deltaTime;
            if (mDyingTimer <= 0.0f) {
                mState = ActorState::Destroy;
            }
        }
    }
    else { // normal movement
        auto* rbc = GetComponent<RigidBodyComponent>();
        if (rbc) {
            rbc->SetVelocity(Vector2(mForwardSpeed, rbc->GetVelocity().y));
        }
    }

    // Destroy if it falls off the bottom of the window
    if (mPosition.y > Game::WINDOW_HEIGHT + Game::TILE_SIZE) {
        mState = ActorState::Destroy;
    }
}

void Goomba::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    // Invert direction upon hitting blocks, enemies, player or collectables
    if (other->GetLayer() == ColliderLayer::Blocks || 
        other->GetLayer() == ColliderLayer::Enemy ||
        other->GetLayer() == ColliderLayer::Player) {
            
        mForwardSpeed *= -1.0f;
        mScale.x *= -1.0f;
    }
    // For collectables: only reverse when it's a Mushroom (coins are ignored)
    else if (other->GetLayer() == ColliderLayer::Collectable) {
        if (dynamic_cast<Mushroom*>(other->GetOwner())) {
            mForwardSpeed *= -1.0f;
            mScale.x *= -1.0f;
        }
    }

    // Kill player upon collision
    if (other->GetLayer() == ColliderLayer::Player) {
        other->GetOwner()->Kill();
    }
}

void Goomba::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    
}
