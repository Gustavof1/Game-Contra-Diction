#pragma once
#include "Actor.h"

class EnemyLaser : public Actor
{
public:
    EnemyLaser(class Game* game);
    
    void OnUpdate(float deltaTime) override;
    
    // Configura a direção e velocidade (ex: 500 px/s)
    void SetVelocity(const Vector2& velocity) { mVelocity = velocity; }

    // Detectar colisão
    void OnHorizontalCollision(float overlap, class AABBColliderComponent* other) override;
    void OnVerticalCollision(float overlap, class AABBColliderComponent* other) override;

private:
    class AABBColliderComponent* mBox;
    Vector2 mVelocity;
    float mLifeTime;
};