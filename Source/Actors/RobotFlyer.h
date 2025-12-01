#pragma once
#include "Actor.h"

class RobotFlyer : public Actor
{
public:
    RobotFlyer(class Game* game);
    ~RobotFlyer();
    void OnUpdate(float deltaTime) override;
    void Kill() override;
    void SetArrival(const Vector2& targetPos);

private:
    void Shoot();
    void PickNewOffset(); // Escolhe um novo ponto aleatório perto do player

    // Componentes
    class AABBColliderComponent* mCollider;
    class ParticleSystemComponent* mLaserParticleSystem;
    class ParticleSystemComponent* mExplosionParticleSystem;

    // Variáveis de Combate
    float mShootCooldown;
    float mAttackRange;

    // Variáveis de Movimento
    Vector2 mCurrentOffset; // Onde eu quero estar RELATIVO ao player agora
    float mMoveTimer;       // Tempo até mudar de posição aleatória
    float mSmoothFactor;    // Quão rápido ele corrige a posição (Lerp)
    float mHoverTimer;      // Timer para animação de hover

    float mInitialY;

    bool mHasActivated;
    float mActivationRange; // Distância para "acordar"

    bool mIsArriving;
    Vector2 mTargetPos; // Onde ele deve estacionar antes de atacar
    float mArrivalSpeed;

    static std::vector<RobotFlyer*> sFlyers;
};