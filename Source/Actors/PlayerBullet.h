#pragma once

#include "Actor.h"

class PlayerBullet : public Actor
{
public:
    PlayerBullet(class Game* game, const Vector2& direction);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;

private:
    void Explode();

    float mLifeTime;
    float mDirection;

    class RectComponent* mRectComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;
};
