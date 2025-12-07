#pragma once
#include "Actor.h"
#include <vector>

class GasCloud : public Actor
{
public:
    GasCloud(class Game* game, const Vector2& direction);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;

private:
    class SpriteComponent* mSprite;
    class AABBColliderComponent* mCollider;
    Vector2 mDirection;
    float mLifeTime;
    float mSpeed;
    std::vector<class Actor*> mHitActors;
};
