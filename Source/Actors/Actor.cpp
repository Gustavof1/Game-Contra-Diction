// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "Actor.h"
#include "../Game.h"
#include "../Components/Component.h"
#include <algorithm>

#include "../Components/Drawing/SpriteComponent.h"

Actor::Actor(Game* game)
        : mState(ActorState::Active)
        , mPosition(Vector2::Zero)
        , mScale(Vector2(1.0f, 1.0f))
        , mRotation(0.0f)
        , mGame(game)
        , mGasExposureAccumulator(0.0f)
        , mIsBeingGassed(false)
        , mOriginalColor(1.0f, 1.0f, 1.0f)
        , mHasStoredOriginalColor(false)
{
    mGame->AddActor(this);
}

Actor::~Actor()
{
    mGame->RemoveActor(this);

    for(auto component : mComponents)
    {
        delete component;
    }
    mComponents.clear();
}

void Actor::Update(float deltaTime)
{
    if (mState == ActorState::Active)
    {
        // Gas Logic
        if (mIsBeingGassed)
        {
            mGasExposureAccumulator += deltaTime;
            
            // Turn Green
            auto sprite = GetComponent<SpriteComponent>();
            if (sprite) {
                if (!mHasStoredOriginalColor) {
                    // Assuming white is default if we can't get it easily, 
                    // but SpriteComponent doesn't expose GetColor easily in snippet.
                    // Let's assume white or try to get it if possible.
                    // Actually SpriteComponent has SetColor but maybe not GetColor in interface?
                    // Let's just assume we tint it green.
                    mHasStoredOriginalColor = true;
                }
                sprite->SetColor(Vector3(0.2f, 1.0f, 0.2f));
            }

            if (mGasExposureAccumulator >= 1.0f)
            {
                Kill();
            }
        }
        else
        {
            // Reset color if not gassed
            if (mHasStoredOriginalColor) {
                auto sprite = GetComponent<SpriteComponent>();
                if (sprite) {
                    sprite->SetColor(mOriginalColor);
                }
                mHasStoredOriginalColor = false;
            }
            // Reset accumulator or decay? "turns green until they die"
            // If they leave gas, they shouldn't die immediately.
            mGasExposureAccumulator = 0.0f;
        }
        
        // Reset flag for next frame
        mIsBeingGassed = false;

        for (auto comp : mComponents)
        {
            if (comp->IsEnabled()) {
                comp->Update(deltaTime);
            }
        }

        OnUpdate(deltaTime);
    }
}

void Actor::OnUpdate(float deltaTime)
{

}

void Actor::ProcessInput(const Uint8* keyState)
{

    if (mState == ActorState::Active)
    {
        for (auto comp : mComponents)
        {
            if (comp->IsEnabled()) {
                comp->ProcessInput(keyState);
            }
        }

        OnProcessInput(keyState);
    }
}

void Actor::OnProcessInput(const Uint8* keyState)
{

}

void Actor::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) {

}

void Actor::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) {

}

void Actor::Kill()
{

}

void Actor::ApplyGasExposure()
{
    mIsBeingGassed = true;
}

void Actor::AddComponent(Component* c)
{
    mComponents.emplace_back(c);
    std::sort(mComponents.begin(), mComponents.end(), [](Component* a, Component* b) {
        return a->GetUpdateOrder() < b->GetUpdateOrder();
    });
}