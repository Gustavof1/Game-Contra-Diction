//
// Created by Vinicius Trindade on 21/10/25.
//

#pragma once
#include "Actor.h"

class Mushroom : public Actor
{
public:
    Mushroom(class Game* game);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void Kill() override;

private:
    float mForwardSpeed;
    bool mIsSpawning;
    Vector2 mSpawnStartPosition;
};