#pragma once
#include "Actor.h"
#include "../Components/ParticleSystemComponent.h"

class RobotTurret : public Actor
{
public:
    RobotTurret(class Game* game);
    
    void OnUpdate(float deltaTime) override;

    // Sobrescrevemos o Kill para fazer a explosão antes de destruir
    void Kill() override;

private:
    void Shoot();
    void Explode();

    float mCooldown;
    float mAttackRange;

    class ParticleSystemComponent* mLaserParticleSystem;
    class ParticleSystemComponent* mExplosionParticleSystem;
};
