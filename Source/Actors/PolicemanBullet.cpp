#include "PolicemanBullet.h"
#include "../Game.h"
#include "../Components/Drawing/RectComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

PolicemanBullet::PolicemanBullet(Game* game, const Vector2& direction)
    : Actor(game)
    , mLifeTime(2.0f)
{
    mRectComponent = new RectComponent(this, 10, 10, RendererMode::TRIANGLES, 200);
    mRectComponent->SetColor(Vector3(1.0f, 0.0f, 0.0f)); // Red bullet

    mRigidBodyComponent = new RigidBodyComponent(this, 0.0f, 0.0f, false); // No gravity
    mRigidBodyComponent->SetVelocity(direction * 400.0f); 

    // Use Enemy layer so it kills player on contact (Player checks collision with Enemy layer)
    mColliderComponent = new AABBColliderComponent(this, 0, 0, 10, 10, ColliderLayer::Enemy, true);
}

void PolicemanBullet::OnUpdate(float deltaTime)
{
    mLifeTime -= deltaTime;
    if (mLifeTime <= 0.0f)
    {
        SetState(ActorState::Destroy);
    }
}

void PolicemanBullet::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Blocks)
    {
        SetState(ActorState::Destroy);
    }
}
