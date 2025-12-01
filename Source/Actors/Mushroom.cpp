//
// Created by Vinicius Trindade on 21/10/25.
//

#include "Mushroom.h"
#include "Spaceman.h"
#include "../Game.h"
#include "../AudioSystem.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

Mushroom::Mushroom(Game* game)
    : Actor(game)
    , mForwardSpeed(200.0f)
    , mIsSpawning(true)
    , mSpawnStartPosition(Vector2::Zero)
{
    GetGame()->GetAudio()->PlaySound("Mushroom.wav");
    
    // Set up animator component
    auto* ac = new AnimatorComponent(this,
                                     "../Assets/Sprites/Collectables/Collectables.png",
                                     "../Assets/Sprites/Collectables/Collectables.json",
                                     Game::TILE_SIZE,
                                     Game::TILE_SIZE,
                                     98);
    ac->AddAnimation("idle", { 1 }); // mushroom frame
    ac->SetAnimation("idle");
    ac->SetAnimFPS(1.0f);
    
    // Set up physics components
    auto* rbc = new RigidBodyComponent(this);
    rbc->SetEnabled(false);

    // Set up collider component and disable it while spawning
    auto* collider = new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE, Game::TILE_SIZE, ColliderLayer::Collectable);
    collider->SetEnabled(false);
}

void Mushroom::OnUpdate(float deltaTime)
{
    if (mIsSpawning) {
        // Initialize spawn start position
        if (mSpawnStartPosition.x == 0.0f && mSpawnStartPosition.y == 0.0f) {
            mSpawnStartPosition = GetPosition();
            SetPosition(Vector2(mSpawnStartPosition.x, mSpawnStartPosition.y - 10));
        }

        // Move up manually since rigid body is disabled
        const float spawnSpeed = 40.0f;
        Vector2 pos = GetPosition();
        pos.y -= spawnSpeed * deltaTime;
        SetPosition(pos);

        // Check if reached final position (one block above start)
        if (GetPosition().y <= mSpawnStartPosition.y - Game::TILE_SIZE) {
            SetPosition(Vector2(mSpawnStartPosition.x, mSpawnStartPosition.y - Game::TILE_SIZE));
            mIsSpawning = false;

            // Enable physics and collider to interact with the world
            GetComponent<RigidBodyComponent>()->SetEnabled(true);
            GetComponent<AABBColliderComponent>()->SetEnabled(true);
        }
    }
    else {
        // Move mushroom forward
        auto* rbc = GetComponent<RigidBodyComponent>();
        if (rbc) {
            rbc->SetVelocity(Vector2(mForwardSpeed, rbc->GetVelocity().y));
        }

        const float halfW = Game::TILE_SIZE * 0.5f;
        const float halfH = Game::TILE_SIZE * 0.5f;
        Vector2 pos = GetPosition();

        // Destroy mushroom if it goes out of bounds
        if (pos.y - halfH > Game::WINDOW_HEIGHT ||
            pos.x + halfW < 0.0f ||
            pos.x - halfW > (Game::LEVEL_WIDTH * Game::TILE_SIZE))
        {
            mState = ActorState::Destroy;
        }
    }
}

void Mushroom::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    // Reverse direction upon hitting blocks or enemies
    if (other->GetLayer() == ColliderLayer::Blocks ||
        other->GetLayer() == ColliderLayer::Enemy) {
        mForwardSpeed *= -1.0f;
    }
}

void Mushroom::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    // Check for collision with player
    if (minOverlap > 0.0f && other->GetLayer() == ColliderLayer::Player) {
    auto* player = dynamic_cast<Spaceman*>(other->GetOwner());
        if (player) {
            player->PowerUp();
        }
        mState = ActorState::Destroy;
    }
}

void Mushroom::Kill()
{
    mState = ActorState::Destroy;
}