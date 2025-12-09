#pragma once
#include "Actor.h"

class CactusProjectile : public Actor
{
public:
    CactusProjectile(class Game* game, class Actor* owner);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;

    class RigidBodyComponent* GetRigidBody() { return mRigidBody; }

private:
    class SpriteComponent* mSprite;
    class AABBColliderComponent* mCollider;
    class RigidBodyComponent* mRigidBody;
    class Actor* mOwner;
    float mLifeTime;
};
