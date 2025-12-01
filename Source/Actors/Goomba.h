//
// Created by Lucas N. Ferreira on 30/09/23.
//

#pragma once

#include "Actor.h"

class Goomba : public Actor
{
public:
    explicit Goomba(Game* game, float forwardSpeed = 100.0f, float deathTime = 0.5f);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, AABBColliderComponent* other) override;
    void setStomped(bool stomped) { mStomped = stomped; }

    void Kill() override;

private:
    bool mIsDying;
    bool mStomped;
    float mForwardSpeed;
    float mDyingTimer;

    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
    class AnimatorComponent* mDrawComponent;
};