#pragma once
#include "Actor.h"

class FinalFlower : public Actor
{
public:
    FinalFlower(class Game* game);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void Kill() override;

private:
    void SpawnBoss(const std::string& reason);

    bool mIsDying;
    bool mIsDancing;
    float mDeathTimer;
    float mIgnoreTimer;
    bool mPlayerSeen;
};
