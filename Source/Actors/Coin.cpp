//
// Created by Vinicius Trindade on 23/10/25.
//

#include "Coin.h"
#include "../Game.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

Coin::Coin(Game* game, bool isAnimatedEffect)
    : Actor(game)
    , mIsAnimatedEffect(isAnimatedEffect)
    , mIsSpawning(true)
    , mIsFalling(false)
    , mSpawnStartPosition(Vector2::Zero)
    , mLifeSpan(0.05f)
{
    auto* ac = new AnimatorComponent(this, "../Assets/Sprites/Collectables/Collectables.png", "../Assets/Sprites/Collectables/Collectables.json", Game::TILE_SIZE, Game::TILE_SIZE, 98);
    ac->AddAnimation("idle", { 0 });
    ac->SetAnimation("idle");
    ac->SetAnimFPS(1.0f);

    // If it's not an animated effect, add a collider
    if (!mIsAnimatedEffect) {
        new AABBColliderComponent(this, 0, 0, Game::TILE_SIZE, Game::TILE_SIZE, ColliderLayer::Collectable, true);
    }
}

void Coin::OnUpdate(float deltaTime)
{
    if (mIsAnimatedEffect) {
        const float travelSpeed = 250.0f;

        if (mIsSpawning) { // Spawning State
            if (mSpawnStartPosition.x == 0.0f && mSpawnStartPosition.y == 0.0f) {
                mSpawnStartPosition = GetPosition();
            }

            Vector2 pos = GetPosition();
            pos.y -= travelSpeed * deltaTime;
            SetPosition(pos);

            // Stop spawning if it reaches the peak
            if (GetPosition().y <= mSpawnStartPosition.y - (Game::TILE_SIZE * 2.0f)) {
                mIsSpawning = false;
                mIsFalling = true;
            }
        } else if (mIsFalling) { // Falling State
            Vector2 pos = GetPosition();
            pos.y += travelSpeed * deltaTime;
            SetPosition(pos);

            // Returned to original position, stop falling
            if (GetPosition().y >= mSpawnStartPosition.y) {
                SetPosition(mSpawnStartPosition);
                mIsFalling = false;
            }
        } else { // Disappearing State
            mLifeSpan -= deltaTime;
            if (mLifeSpan <= 0.0f) {
                mState = ActorState::Destroy;
            }
        }
    }
}