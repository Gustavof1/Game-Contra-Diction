//
// Created by Lucas N. Ferreira on 18/09/25.
//

#pragma once

#include "../Actors/Actor.h"
#include "Component.h"
#include <vector>
#include <functional>
#include <string>

#include "Physics/AABBColliderComponent.h"

class Particle : public Actor
{
public:
    Particle(class Game* game, int width, int height, const std::string& texturePath, const std::string& jsonPath, ColliderLayer layer, bool useGravity = false, float colliderScale = 0.5f, const Vector2& drawOffset = Vector2::Zero);

    void OnUpdate(float deltaTime) override;
    void OnHorizontalCollision(const float minOverlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(const float minOverlap, class AABBColliderComponent* other) override;

    bool IsDead() const { return mIsDead; }
    void Awake(const Vector2 &position, float rotation, float lifetime = 1.0f);
    void Kill() override;

    void SetOnCollision(std::function<void(class AABBColliderComponent*)> callback) { mOnCollision = callback; }
    class AnimatorComponent* GetAnimator() { return mAnimator; }

private:
    float mLifeTime;
    bool mIsDead;
    bool mIsExploding;

    class AnimatorComponent* mAnimator;
    class RigidBodyComponent* mRigidBodyComponent;
    class AABBColliderComponent* mColliderComponent;

    std::function<void(class AABBColliderComponent*)> mOnCollision;
};

class ParticleSystemComponent : public Component {

public:
    ParticleSystemComponent(class Actor* owner, int partibleW, int particleH, 
                            const std::string& texturePath, const std::string& jsonPath,
                            int poolSize = 100, int updateOrder = 10, ColliderLayer collisionLayer = ColliderLayer::PlayerProjectile,
                            bool useGravity = false, float colliderScale = 0.5f, const Vector2& drawOffset = Vector2::Zero);
    void EmitParticle(float lifetime, float speed, const Vector2& direction, const Vector2& offsetPosition = Vector2::Zero);
    
    void SetCollisionCallback(std::function<void(class AABBColliderComponent*)> callback);

private:
    std::vector<class Particle*> mParticles;
};
