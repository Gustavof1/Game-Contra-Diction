#include "EnemyLaser.h"
#include "Spaceman.h"
#include "../Game.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

EnemyLaser::EnemyLaser(Game* game, Actor* shooter)
    :Actor(game)
    ,mLifeTime(3.0f)
    ,mVelocity(Vector2::Zero)
    ,mShooter(shooter)
{
    // 1. Sprite
    SpriteComponent* sc = new SpriteComponent(this, 200);
    // Placeholder colored square
    Texture* tex = game->GetRenderer()->GetTexture("../Assets/Sprites/Blocks/BlockA.png");
    sc->SetTexture(tex);
    sc->SetColor(Vector3(1.0f, 0.0f, 0.0f)); // Red

    // 2. Colisor (TRIGGER)
    // Pequeno (8x8) para não ser injusto
    mBox = new AABBColliderComponent(this, 0, 0, 8, 8, ColliderLayer::EnemyProjectile, true, false);
}

void EnemyLaser::OnUpdate(float deltaTime)
{
    // Mover
    Vector2 pos = GetPosition();
    pos.x += mVelocity.x * deltaTime;
    pos.y += mVelocity.y * deltaTime;
    SetPosition(pos);

    // Tempo de vida
    mLifeTime -= deltaTime;
    if (mLifeTime <= 0.0f) {
        SetState(ActorState::Destroy);
    }
}

void EnemyLaser::OnHorizontalCollision(float overlap, AABBColliderComponent* other)
{
    // Se bateu no Player
    Spaceman* player = dynamic_cast<Spaceman*>(other->GetOwner());
    if (player) {
        Actor* killer = this;
        if (mShooter) {
            killer = mShooter;
        }
        GetGame()->SetGameOverInfo(killer);
        player->Kill();
        SetState(ActorState::Destroy); // Laser some
    }
    // Se bateu na parede (e não é outro trigger tipo espinho)
    else if (!other->IsTrigger()) {
        SetState(ActorState::Destroy);
    }
}

void EnemyLaser::OnVerticalCollision(float overlap, AABBColliderComponent* other)
{
    OnHorizontalCollision(overlap, other); // Mesma lógica
}