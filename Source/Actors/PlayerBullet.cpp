#include "PlayerBullet.h"
#include "Goomba.h"
#include "../Game.h"
#include "../Math.h"
#include "../Components/Drawing/RectComponent.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

PlayerBullet::PlayerBullet(Game* game, const Vector2& direction)
    : Actor(game)
    , mLifeTime(1.2f)
    , mDirection(0.0f) // Unused now
    , mRectComponent(nullptr)
    , mRigidBodyComponent(nullptr)
    , mColliderComponent(nullptr)
{
    SpriteComponent* sc = new SpriteComponent(this, 200);

    Texture* tex = game->GetRenderer()->GetTexture("../Assets/Sprites/Bullet.png");

    sc->SetTexture(tex);

    mRigidBodyComponent = new RigidBodyComponent(this, 0.2f, 0.0f, false);
    mRigidBodyComponent->SetVelocity(direction * 1600.0f);

    SetRotation(Math::Atan2(direction.y, direction.x));

    mColliderComponent = new AABBColliderComponent(this, 0, 0, 32, 12, ColliderLayer::PlayerProjectile, true);
}

void PlayerBullet::OnUpdate(float deltaTime)
{
    mLifeTime -= deltaTime;
    if (mLifeTime <= 0.0f)
    {
        Explode();
        return;
    }

    const float levelWidth = Game::LEVEL_WIDTH * Game::TILE_SIZE;
    if (mPosition.x < -Game::TILE_SIZE || mPosition.x > levelWidth + Game::TILE_SIZE)
    {
        Explode();
    }
}

void PlayerBullet::Explode()
{
    mState = ActorState::Destroy;
}

void PlayerBullet::OnHorizontalCollision(const float /*minOverlap*/, AABBColliderComponent* other)
{
    if (!other)
    {
        return;
    }

    if (other->GetLayer() == ColliderLayer::Enemy || other->GetLayer() == ColliderLayer::Destructible)
    {
        other->GetOwner()->Kill();
        Explode();
    }
    else if (other->GetLayer() == ColliderLayer::Blocks)
    {
        Explode();
    }
}

void PlayerBullet::OnVerticalCollision(const float /*minOverlap*/, AABBColliderComponent* other)
{
    if (!other) {
        return;
    }

    if (other->GetLayer() == ColliderLayer::Enemy || other->GetLayer() == ColliderLayer::Blocks || other->GetLayer() == ColliderLayer::Destructible) {
        if (other->GetLayer() == ColliderLayer::Enemy || other->GetLayer() == ColliderLayer::Destructible) {
            other->GetOwner()->Kill();
        }
        Explode();
    }
}
