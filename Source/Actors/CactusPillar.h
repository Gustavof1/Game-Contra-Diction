#pragma once
#include "Actor.h"

class CactusPillar : public Actor
{
public:
    CactusPillar(class Game* game, const Vector2& pos);
    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;

private:
    class SpriteComponent* mSprite;
    class AABBColliderComponent* mCollider;
    class RigidBodyComponent* mRigidBody;
    
    float mLifeTime;
    float mRiseSpeed;
    float mTargetY;
    bool mRising;
    
    // Warning Phase
    float mWarningTimer;
    bool mIsWarning;
};
