#include "RobotTurret.h"
#include "EnemyLaser.h"
#include "../Game.h"
#include "Spaceman.h" // Para saber onde o player está
#include "../Components/ParticleSystemComponent.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

RobotTurret::RobotTurret(Game* game)
    :Actor(game)
    ,mCooldown(0.0f)
    ,mAttackRange(400.0f) // Alcance de 400 pixels
{
    // 1. Sprite
    // A textura será carregada no BuildLevelFromJSON, igual à árvore/vegetação
    SpriteComponent* sc = new SpriteComponent(this);
    sc->SetTexture(game->GetRenderer()->GetTexture("../Assets/Sprites/Drones-ContraDiction/landdrone.png"));

    new AABBColliderComponent(this, 0, 0, 48, 32, ColliderLayer::Enemy, false, true);

    SetState(ActorState::Active);

    mLaserParticleSystem = new ParticleSystemComponent(this,
    16, 16,
    "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.png",
        "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.json",
    20,
    100,
    ColliderLayer::EnemyProjectile
);

    mExplosionParticleSystem = new ParticleSystemComponent(this, 
        64, 64, // Even Bigger debris
        "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.png",
        "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.json",
        20, 
        100, 
        ColliderLayer::EnemyProjectile,
        false, // Gravity disabled for debris
        0.5f,
        Vector2(16.0f, 16.0f) // Shift for 64x64
    );

    // 2. Callback de colisão
    mLaserParticleSystem->SetCollisionCallback([this](AABBColliderComponent* other) {
        // Se bater no Player
        if (other->GetLayer() == ColliderLayer::Player)
        {
            auto player = dynamic_cast<Spaceman*>(other->GetOwner());
            if (player) {
                GetGame()->SetGameOverInfo(this);
                player->Kill();
            }
        }
    });
}

void RobotTurret::OnUpdate(float deltaTime)
{
    mCooldown -= deltaTime;

    // LOG 1: Saber se o Update está rodando (O robô está vivo?)
    // SDL_Log("Update do Robo rodando...");

    const Spaceman* player = GetGame()->GetPlayer();

    if (!player) {
        SDL_Log("Robo: Nao encontrei o player!");
        return;
    }

    if (player) {
        Vector2 myPos = GetPosition();
        Vector2 playerPos = player->GetPosition();

        // Vetor distância
        Vector2 diff = playerPos - myPos;
        
        // Verifica distância (usando quadrado para performance)
        if (diff.LengthSq() < mAttackRange * mAttackRange) {
            
            // Lógica visual: Virar para o player
            if (diff.x < 0) {
                SetScale(Vector2(-1, 1)); // Esquerda (Flip se sprite original olha pra direita)
            } else {
                SetScale(Vector2(1, 1)); // Direita
            }

            // Atirar
            if (mCooldown <= 0.0f) {
                Shoot();
                mCooldown = 2.0f; // Atira a cada 2 segundos
            }
        }
    }
}

void RobotTurret::Shoot()
{
    const Spaceman* player = GetGame()->GetPlayer();
    if (!player) return;

    // 1. Calcular Direção Normalizada
    Vector2 myPos = GetPosition();
    Vector2 targetPos = player->GetPosition();
    
    Vector2 direction = targetPos - myPos;
    direction.Normalize();

    float sign = GetScale().x;

    // O EmitParticle usa o offset relativo ao dono.
    // Como queremos 32px na direção do tiro:
    Vector2 offset = (direction * 32.0f) * sign;

    // Dispara! (Tempo de vida 3s, Velocidade 300)
    mLaserParticleSystem->EmitParticle(3.0f, 300.0f, direction, offset);
}

void RobotTurret::Kill() {
    if (GetState() == ActorState::Destroy) return;

    GetGame()->OnNPCKilled(this);
    Explode();

    SetState(ActorState::Destroy);
}

void RobotTurret::Explode() {
    // Lança partículas em todas as direções usando o sistema que já temos
    for (int i = 0; i < 30; i++) {
        // Random direction
        float angle = (rand() % 360) * (3.14159f / 180.0f);
        float speed = 150.0f + (rand() % 250);
        Vector2 dir(cos(angle), sin(angle));
        
        // Random lifetime for "fade out" effect (staggered disappearance)
        float lifetime = 0.5f + (rand() % 150) / 100.0f; // 0.5 to 2.0 seconds

        mExplosionParticleSystem->EmitParticle(lifetime, speed, dir);
    }
}
