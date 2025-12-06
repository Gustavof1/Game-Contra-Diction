#include "GasCloud.h"
#include "../Game.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Random.h"

GasCloud::GasCloud(Game* game, const Vector2& direction)
    : Actor(game)
    , mDirection(direction)
    , mLifeTime(1.0f)
    , mSpeed(400.0f)
{
    mSprite = new SpriteComponent(this, 150);
    // Use procedural circle instead of texture
    mSprite->SetIsCircle(true);
    // Set a base size for the circle since we don't have a texture
    mSprite->SetTextureRect(0, 0, 32, 32);
    
    // Greenish gas color with 30% opacity for better layering
    mSprite->SetColor(Vector3(0.2f, 0.8f, 0.2f));
    mSprite->SetAlpha(0.3f);

    // Random rotation
    SetRotation(Random::GetFloatRange(0.0f, Math::Pi * 2.0f));
    
    // Random scale variation (smaller particles)
    float scale = Random::GetFloatRange(0.8f, 1.5f);
    SetScale(Vector2(scale, scale));

    // Collider for precise collision
    mCollider = new AABBColliderComponent(this, 0, 0, 20, 20, ColliderLayer::PlayerProjectile);
}

void GasCloud::OnUpdate(float deltaTime)
{
    mLifeTime -= deltaTime;
    if (mLifeTime <= 0.0f)
    {
        SetState(ActorState::Destroy);
        return;
    }

    // Move
    Vector2 pos = GetPosition();
    pos += mDirection * mSpeed * deltaTime;
    SetPosition(pos);
    
    // Rotate slowly
    SetRotation(GetRotation() + deltaTime);
}

void GasCloud::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        other->GetOwner()->ApplyGasExposure();
    }
}

void GasCloud::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    OnHorizontalCollision(minOverlap, other);
}
