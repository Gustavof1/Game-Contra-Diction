#include "SoldierBullet.h"
#include "../Game.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"

SoldierBullet::SoldierBullet(Game* game, const Vector2& direction, Actor* shooter)
    : Actor(game)
    , mLifeTime(3.0f)
    , mShooter(shooter)
{
    mAnimatorComponent = new AnimatorComponent(this, 
        "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.png",
        "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.json",
        32, 32, 200);
    
    // Add an animation that uses all frames
    std::vector<int> frames;
    size_t count = mAnimatorComponent->GetFrameCount();
    for(size_t i=0; i<count; ++i) frames.push_back(i);
    mAnimatorComponent->AddAnimation("idle", frames);
    mAnimatorComponent->SetAnimation("idle");
    mAnimatorComponent->SetAnimFPS(10.0f);
    
    // Scale to match player bullet visual size if needed (Spaceman uses 48x48)
    mAnimatorComponent->SetSize(48.0f, 48.0f);

    mRigidBodyComponent = new RigidBodyComponent(this, 0.0f, 0.0f, false); // No gravity
    mRigidBodyComponent->SetVelocity(direction * 400.0f); 

    // Use Enemy layer so it kills player on contact
    mColliderComponent = new AABBColliderComponent(this, 0, 0, 12, 12, ColliderLayer::Enemy, true);
}

void SoldierBullet::OnUpdate(float deltaTime)
{
    mLifeTime -= deltaTime;
    if (mLifeTime <= 0.0f)
    {
        SetState(ActorState::Destroy);
    }
}

void SoldierBullet::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    // Pierce everything: do nothing on collision
}
