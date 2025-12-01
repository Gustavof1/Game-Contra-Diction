#include "Vine.h"
#include "../Game.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "Spaceman.h"

Vine::Vine(Game* game)
    :Actor(game)
    ,mLifeTime(2.0f)
    ,mSpeed(300.0f)
{
    SpriteComponent* sc = new SpriteComponent(this);
    // Using BlockA.png as placeholder
    sc->SetTexture(game->GetRenderer()->GetTexture("../Assets/Sprites/Blocks/BlockA.png"));
    sc->SetColor(Vector3(0.0f, 1.0f, 0.0f)); // Green
    
    // Collider
    mBox = new AABBColliderComponent(this, 0, 0, 32, 32, ColliderLayer::EnemyProjectile);
}

void Vine::OnUpdate(float deltaTime)
{
    // Move up
    Vector2 pos = GetPosition();
    
    // Move up for the first 0.5 seconds, then stop
    if (mLifeTime > 1.5f) {
         pos.y -= mSpeed * deltaTime;
    }
    
    SetPosition(pos);
    
    mLifeTime -= deltaTime;
    if (mLifeTime <= 0.0f) {
        SetState(ActorState::Destroy);
    }
}

void Vine::OnHorizontalCollision(float overlap, AABBColliderComponent* other)
{
    Spaceman* player = dynamic_cast<Spaceman*>(other->GetOwner());
    if (player) {
        player->Kill();
    }
}

void Vine::OnVerticalCollision(float overlap, AABBColliderComponent* other)
{
    OnHorizontalCollision(overlap, other);
}
