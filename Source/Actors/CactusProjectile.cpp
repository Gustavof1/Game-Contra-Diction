#include "CactusProjectile.h"
#include "../Game.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "Spaceman.h"

CactusProjectile::CactusProjectile(Game* game, Actor* owner)
    : Actor(game)
    , mOwner(owner)
    , mLifeTime(0.0f)
{
    mSprite = new SpriteComponent(this);
    mSprite->SetTexture(game->GetRenderer()->GetTexture("../Assets/Sprites/Cactus/cactus_projectile.PNG"));
    mSprite->SetIsCircle(true); // Assuming it's round-ish
    SetScale(Vector2(0.3f, 0.3f)); // Even Smaller

    // Reduced collider size to 20x20 for better accuracy
    mCollider = new AABBColliderComponent(this, 0, 0, 20, 20, ColliderLayer::Enemy);
    
    // Set friction in constructor (mass=1.0f, friction=0.5f)
    mRigidBody = new RigidBodyComponent(this, 1.0f, 0.5f);
    mRigidBody->SetApplyGravity(true);
}

void CactusProjectile::OnUpdate(float deltaTime)
{
    mLifeTime += deltaTime;
    
    // Rotate while moving
    float velX = mRigidBody->GetVelocity().x;
    SetRotation(GetRotation() + velX * deltaTime * 0.1f);

    if (mLifeTime > 5.0f) {
        SetState(ActorState::Destroy);
    }
}

void CactusProjectile::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Player) {
        Spaceman* player = dynamic_cast<Spaceman*>(other->GetOwner());
        if (player) {
            GetGame()->SetGameOverInfo(mOwner ? mOwner : this);
            player->Kill();
            SetState(ActorState::Destroy);
            // Explosion effect?
        }
    } else if (other->GetLayer() == ColliderLayer::Blocks) {
        // Bounce or explode? User said "explode when it reaches the ground or when it touch the player"
        // If horizontal collision with block, maybe explode too?
        SetState(ActorState::Destroy);
    }
}

void CactusProjectile::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Player) {
        Spaceman* player = dynamic_cast<Spaceman*>(other->GetOwner());
        if (player) {
            GetGame()->SetGameOverInfo(mOwner ? mOwner : this);
            player->Kill();
            SetState(ActorState::Destroy);
        }
    } else if (other->GetLayer() == ColliderLayer::Blocks) {
        // Explode on ground
        SetState(ActorState::Destroy);
    }
}
