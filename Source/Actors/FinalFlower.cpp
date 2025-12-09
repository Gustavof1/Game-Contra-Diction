#include "FinalFlower.h"
#include "FlowerBoss.h"
#include "../Game.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "Spaceman.h"
#include "PlayerBullet.h"

FinalFlower::FinalFlower(Game* game)
    : Actor(game)
    , mIsDying(false)
    , mIsDancing(false)
    , mDeathTimer(1.0f)
    , mIgnoreTimer(5.0f)
    , mPlayerSeen(false)
{
    // Setup Animator
    auto* ac = new AnimatorComponent(this, "../Assets/Sprites/FinalFlower/a_mini_bee_flying_on_a_flower_dancing_fower_default.png", "../Assets/Sprites/FinalFlower/a_mini_bee_flying_on_a_flower_dancing_fower_default.json", 107, 128, 90);
    ac->AddAnimation("idle", {0});
    
    std::vector<int> danceFrames;
    for(int i=0; i<48; i++) danceFrames.push_back(i);
    ac->AddAnimation("dance", danceFrames);
    
    // Death animation
    size_t deathStart = ac->AppendSpriteSheet(
        "../Assets/Sprites/FinalFlower/a_mini_bee_flying_on_a_flower_death_default.png",
        "../Assets/Sprites/FinalFlower/a_mini_bee_flying_on_a_flower_death_default.json"
    );
    std::vector<int> deathFrames;
    for(int i=0; i<48; i++) deathFrames.push_back(deathStart + i);
    ac->AddAnimation("death", deathFrames);

    ac->SetAnimation("idle");

    // Setup Collider
    new AABBColliderComponent(this, 0, 0, 107, 128, ColliderLayer::Enemy);
}

void FinalFlower::OnUpdate(float deltaTime)
{
    if (mIsDying) {
        auto* ac = GetComponent<AnimatorComponent>();
        if (ac && ac->IsFinished()) {
            SpawnBoss("kill");
            SetState(ActorState::Destroy);
        }
        return;
    }

    auto player = GetGame()->GetPlayer();
    if (player) {
        Vector2 diff = player->GetPosition() - GetPosition();
        float dist = diff.Length();

        if (dist < 300.0f) {
            mPlayerSeen = true;
            
            if (player->IsDancing()) {
                if (!mIsDancing) {
                    mIsDancing = true;
                    auto* ac = GetComponent<AnimatorComponent>();
                    ac->SetAnimation("dance");
                    // Disable collider so player can pass
                    auto* col = GetComponent<AABBColliderComponent>();
                    if (col) col->SetEnabled(false);

                    GetGame()->AddFloatingText(GetPosition(), "Gostei de voce.", 3.0f, this);
                }
            }
        } else if (mPlayerSeen && dist > 400.0f && !mIsDancing) {
             SpawnBoss("ignore");
             SetState(ActorState::Destroy);
        }
    }
}

void FinalFlower::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::PlayerProjectile) {
        Kill();
        other->GetOwner()->SetState(ActorState::Destroy);
    }
}

void FinalFlower::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Player) {
        if (other->GetOwner()->GetPosition().y < GetPosition().y) {
             Kill();
             auto rb = other->GetOwner()->GetComponent<RigidBodyComponent>();
             if (rb) rb->SetVelocity(Vector2(rb->GetVelocity().x, -700.0f));
        }
    }
}

void FinalFlower::Kill()
{
    if (!mIsDying) {
        mIsDying = true;
        auto* ac = GetComponent<AnimatorComponent>();
        ac->SetAnimation("death");
        ac->SetLooping(false);
    }
}

void FinalFlower::SpawnBoss(const std::string& reason)
{
    auto* boss = new FlowerBoss(GetGame(), reason);
    boss->SetPosition(GetPosition());
}
