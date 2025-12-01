#pragma once
#include "Actor.h"

class Vine : public Actor
{
public:
    Vine(class Game* game);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(float overlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(float overlap, class AABBColliderComponent* other) override;
private:
    class AABBColliderComponent* mBox;
    float mLifeTime;
    float mSpeed;
};
