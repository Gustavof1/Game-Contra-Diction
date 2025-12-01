//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once
#include "../Component.h"
#include "../../Math.h"
#include "RigidBodyComponent.h"
#include <vector>
#include <set>
#include <functional>

using CollisionCallback = std::function<void(class AABBColliderComponent*)>;

enum class ColliderLayer
{
    Player,
    Enemy,
    Hazard,
    Blocks,
    Collectable,
    PlayerProjectile,
    EnemyProjectile,
    Destructible
};

class AABBColliderComponent : public Component
{
public:

    AABBColliderComponent(class Actor* owner, int dx, int dy, int w, int h, ColliderLayer layer, 
                            bool isTrigger = false, bool isStatic = false, int updateOrder = 10);
    ~AABBColliderComponent() override;

    bool Intersect(const AABBColliderComponent& b) const;

    float DetectHorizontalCollision(RigidBodyComponent *rigidBody);
    float DetectVertialCollision(RigidBodyComponent *rigidBody);

    Vector2 GetMin() const;
    Vector2 GetMax() const;
    ColliderLayer GetLayer() const { return mLayer; }
    void SetLayer(ColliderLayer layer) { mLayer = layer; }

    void SetSize(float width, float height);
    void SetOffset(const Vector2& offset);

    bool IsTrigger() const { return mIsTrigger; }

    // Drawing for debug purposes
    void DebugDraw(class Renderer* renderer) override;

    void SetCollisionCallback(CollisionCallback callback) { mCollisionCallback = callback; }

    // Função que será chamada pelo Game.cpp a cada frame para checar triggers
    void Update(float deltaTime) override;

private:
    float GetMinVerticalOverlap(AABBColliderComponent* b) const;
    float GetMinHorizontalOverlap(AABBColliderComponent* b) const;

    void ResolveHorizontalCollisions(RigidBodyComponent *rigidBody, const float minOverlap);
    void ResolveVerticalCollisions(RigidBodyComponent *rigidBody, const float minOverlap);

    Vector2 mOffset;
    int mWidth;
    int mHeight;
    bool mIsStatic;
    bool mIsTrigger;

    ColliderLayer mLayer;

    CollisionCallback mCollisionCallback;
};