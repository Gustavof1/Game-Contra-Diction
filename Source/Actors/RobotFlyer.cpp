#include "RobotFlyer.h"
#include "../Game.h"
#include "Spaceman.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Components/ParticleSystemComponent.h"
#include <algorithm>

std::vector<RobotFlyer*> RobotFlyer::sFlyers;

RobotFlyer::RobotFlyer(Game* game)
    :Actor(game)
    ,mShootCooldown(1.5f)
    ,mAttackRange(600.0f) // Alcance maior que a Turret
    ,mMoveTimer(0.0f)
    ,mSmoothFactor(2.0f)  // Ajuste isso: Maior = mais rápido, Menor = mais "bóia"
    ,mHoverTimer(0.0f)
    ,mInitialY(0.0f)      // Inicializa com 0
    ,mHasActivated(false)
    ,mActivationRange(700.0f)
    ,mIsArriving(false)
    ,mArrivalSpeed(250.0f)
{
    // 1. Sprite (Voador)
    SpriteComponent* sc = new SpriteComponent(this);
    sc->SetTexture(game->GetRenderer()->GetTexture("../Assets/Sprites/Drones-ContraDiction/flyingdrone1.png"));

    // 2. Colisor (Não usamos RigidBody para voadores simples, movemos manual)
    new AABBColliderComponent(this, 0, 0, 32, 24, ColliderLayer::Enemy, false, true);

    SetState(ActorState::Active);

    // 3. Sistema de Tiro (Mesmo JSON do laser vermelho)
    mLaserParticleSystem = new ParticleSystemComponent(this, 
    32, 32,
"../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.png",
    "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.json",
        20, 
        100, 
        ColliderLayer::EnemyProjectile,
        false, // No gravity
        0.25f,  // Small collider (32 * 0.25 = 8px)
        Vector2(8.0f, 8.0f) // Shift sprite to bottom-right to center the visual bullet
    );

    mExplosionParticleSystem = new ParticleSystemComponent(this, 
        64, 64, // Even Bigger debris
        "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.png",
        "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.json",
        20, 
        100, 
        ColliderLayer::EnemyProjectile,
        true, // Gravity enabled for debris
        0.5f,
        Vector2(16.0f, 16.0f) // Shift for 64x64
    );

    mLaserParticleSystem->SetCollisionCallback([this](AABBColliderComponent* other) {
        if (other->GetLayer() == ColliderLayer::Player) {
            auto player = dynamic_cast<Spaceman*>(other->GetOwner());
            if (player) {
                GetGame()->SetGameOverInfo(this);
                player->Kill();
            }
        }
    });

        // Inicializa o primeiro movimento
    PickNewOffset();

    sFlyers.push_back(this);
}

RobotFlyer::~RobotFlyer()
{
    auto it = std::find(sFlyers.begin(), sFlyers.end(), this);
    if (it != sFlyers.end())
    {
        sFlyers.erase(it);
    }
}

void RobotFlyer::OnUpdate(float deltaTime)
{
    if (mIsArriving) {
        Vector2 currentPos = GetPosition();

        // Calcula vetor direção até o alvo
        Vector2 diff = mTargetPos - currentPos;
        float dist = diff.Length(); // Assumindo que sua classe Vector2 tem Length()

        // Se chegou perto o suficiente (ex: 10 pixels), ativa a IA normal
        if (dist < 5.0f) {
            mIsArriving = false;
        } else {
            diff.Normalize();
            SetPosition(currentPos + diff * mArrivalSpeed * deltaTime);
        }

        return;
    }

    if (mInitialY == 0.0f) {
        mInitialY = GetPosition().y;

        // Se quiser forçar ele a subir IMEDIATAMENTE ao nascer:
        // mInitialY -= 150.0f; // Sobe 150px do ponto de spawn
    }

    const Spaceman* player = GetGame()->GetPlayer();
    if (!player) return;

    Vector2 playerPos = player->GetPosition();
    Vector2 myPos = GetPosition();
    float distSq = (playerPos - myPos).LengthSq();

    // =========================================================
    // LÓGICA DE "ACORDAR"
    // =========================================================
    if (!mHasActivated)
    {
        // Se ainda não ativou, verifica se o player chegou perto
        if (distSq < mActivationRange * mActivationRange)
        {
            mHasActivated = true;
            GetGame()->GetAudio()->PlaySound("DroneActive.wav");
            // Opcional: Tocar um som de "Drone ativando" aqui
        }
        else
        {
            // Se está longe, NÃO FAZ NADA (Return).
            // Ele fica parado na posição original do Tiled.
            return;
        }
    }

    // 1. Timer do movimento aleatório
    mMoveTimer -= deltaTime;
    if (mMoveTimer <= 0.0f) {
        PickNewOffset();
    }

    mHoverTimer += deltaTime;

    // =========================================================
    // NOVA LÓGICA DE MOVIMENTO
    // =========================================================

    // EIXO X: Segue o Player (com o offset aleatório para não ficar grudado)
    float targetX;
    if (player->GetPosture() == PlayerPosture::Crouching) {
        targetX = GetPosition().x;
    } else {
        targetX = playerPos.x + mCurrentOffset.x;
    }

    // EIXO Y: Usa a Altura Inicial + pequena flutuação (IGNORA O PLAYER)
    // mCurrentOffset.y agora serve apenas para ele "bóiar" um pouco para cima e para baixo
    float hoverOffset = 10.0f * sinf(mHoverTimer * 5.0f);
    float targetY = mInitialY + mCurrentOffset.y + hoverOffset;

    Vector2 targetPos(targetX, targetY);

    // =========================================================
    // FLOCKING LOGIC
    // =========================================================
    Vector2 separationForce = Vector2::Zero;
    Vector2 cohesionForce = Vector2::Zero;
    Vector2 centerOfMass = Vector2::Zero;
    int neighborCount = 0;

    float separationRadius = 80.0f;
    float neighborRadius = 300.0f;
    
    for (auto* other : sFlyers) {
        if (other == this) continue;
        if (other->GetState() != ActorState::Active) continue;

        Vector2 toMe = myPos - other->GetPosition();
        float distSq = toMe.LengthSq();
        
        if (distSq < neighborRadius * neighborRadius) {
            // Cohesion
            centerOfMass += other->GetPosition();
            neighborCount++;

            // Separation
            if (distSq < separationRadius * separationRadius && distSq > 0.001f) {
                float dist = sqrt(distSq);
                // Force is stronger when closer
                Vector2 pushDir = toMe * (1.0f / dist);
                separationForce += pushDir * (1.0f - dist / separationRadius);
            }
        }
    }

    if (neighborCount > 0) {
        centerOfMass *= (1.0f / neighborCount);
        Vector2 toCenter = centerOfMass - myPos;
        if (toCenter.LengthSq() > 0.001f) {
            toCenter.Normalize();
            cohesionForce = toCenter;
        }
    }

    if (separationForce.LengthSq() > 0.001f) {
        separationForce.Normalize();
    }

    // Apply Forces to Target Position
    // We modify where we want to go based on flocking
    Vector2 flockAdjustedTarget = targetPos;
    
    // Cohesion: Pull towards center of flock
    flockAdjustedTarget += (cohesionForce * 50.0f); 
    
    // Separation: Push away from neighbors (Stronger influence)
    flockAdjustedTarget += (separationForce * 150.0f);

    // Lerp suave towards adjusted target
    Vector2 smoothMove = myPos + (flockAdjustedTarget - myPos) * mSmoothFactor * deltaTime;

    // Limit to screen bounds
    Vector2 cameraPos = GetGame()->GetCameraPos();
    float padding = 32.0f; // Keep it slightly inside

    // Limit Left
    if (smoothMove.x < cameraPos.x + padding) {
        smoothMove.x = cameraPos.x + padding;
    }
    // Limit Top
    if (smoothMove.y < cameraPos.y + padding) {
        smoothMove.y = cameraPos.y + padding;
    }
    
    SetPosition(smoothMove);

    // Virar sprite (Olhar para o player)
    if (targetX < myPos.x) SetScale(Vector2(1, 1));
    else SetScale(Vector2(-1, 1));

    // =========================================================
    // LÓGICA DE TIRO (Continua Igual)
    // =========================================================
    mShootCooldown -= deltaTime;

    // Aumente um pouco a tolerância de tiro vertical já que ele está mais alto
    Vector2 diff = playerPos - myPos;

    // Só atira se estiver perto horizontalmente (não atira se estiver muito longe na tela)
    if (player->GetPosture() != PlayerPosture::Crouching && abs(diff.x) < 500.0f) {
        if (mShootCooldown <= 0.0f) {
            Shoot();
            mShootCooldown = 2.0f + (rand() % 100) / 100.0f;
        }
    }
}

void RobotFlyer::PickNewOffset()
{
    // A cada 2 ou 3 segundos, mudamos o ponto de destino
    mMoveTimer = 2.0f + (rand() % 100) / 50.0f;

    // REGRAS DE POSICIONAMENTO:
    // Altura: Entre 100 e 200 pixels ACIMA do player (Y negativo é pra cima)
    float randomY = -100.0f - (rand() % 100); 

    // Distância: Entre 150 e 250 pixels de distância horizontal
    // Vamos decidir aleatoriamente se ficamos na esquerda ou direita
    float side = (rand() % 2 == 0) ? 1.0f : -1.0f;
    float randomX = (150.0f + (rand() % 100)) * side;

    mCurrentOffset = Vector2(randomX, randomY);
}

void RobotFlyer::Shoot()
{
    // Copie exatamente a função Shoot do RobotTurret aqui
    // Lembre-se do truque do scale * -1 que fizemos antes!
    const Spaceman* player = GetGame()->GetPlayer();
    if (!player) return;

    Vector2 myPos = GetPosition();
    Vector2 targetPos = player->GetPosition();
    Vector2 direction = targetPos - myPos;
    direction.Normalize(); 

    float sign = GetScale().x; 
    Vector2 offset = (direction * 24.0f) * sign; // Offset um pouco menor pq drone é pequeno

    mLaserParticleSystem->EmitParticle(3.0f, 300.0f, direction, offset);
}

void RobotFlyer::Kill()
{
    if (GetState() == ActorState::Destroy) return;
    
    GetGame()->GetAudio()->PlaySound("Bigboom.wav");

    // Efeito de explosão
    for (int i = 0; i < 30; i++) {
        // Random direction
        float angle = (rand() % 360) * (3.14159f / 180.0f);
        float speed = 150.0f + (rand() % 250);
        Vector2 dir(cos(angle), sin(angle));
        
        // Random lifetime for "fade out" effect (staggered disappearance)
        float lifetime = 0.5f + (rand() % 150) / 100.0f; // 0.5 to 2.0 seconds

        mExplosionParticleSystem->EmitParticle(lifetime, speed, dir);
    }
    
    SetState(ActorState::Destroy);
}

void RobotFlyer::SetArrival(const Vector2& targetPos)
{
    mIsArriving = true;
    mTargetPos = targetPos;

    float randomOffsetX = static_cast<float>(rand() % 200 - 100);

    Vector2 startPos;
    startPos.x = targetPos.x + randomOffsetX;
    startPos.y = targetPos.y - 600.0f;

    SetPosition(startPos);
}