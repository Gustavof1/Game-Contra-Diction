#include "CactusPillar.h"
#include "../Game.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Random.h"
#include "Spaceman.h"

CactusPillar::CactusPillar(Game* game, const Vector2& pos)
    : Actor(game)
    , mLifeTime(0.0f)
    , mRiseSpeed(100.0f) // Slower
    , mRising(true)
    , mWarningTimer(1.5f)
    , mIsWarning(true)
{
    // Start below ground (adjusted for scale)
    // Original height ~128, scale 0.7 -> ~90
    SetPosition(Vector2(pos.x, pos.y + 100.0f)); 
    mTargetY = pos.y - 30.0f; // Go up a little more
    SetScale(Vector2(0.7f, 0.7f));

    // Cactus Sprite (Initially hidden or just below ground)
    mSprite = new SpriteComponent(this, 110); 
    int variant = Random::GetIntRange(1, 3);
    std::string texturePath = "../Assets/Sprites/Cactus/cactus_pillar_" + std::to_string(variant) + ".PNG";
    mSprite->SetTexture(game->GetRenderer()->GetTexture(texturePath));
    
    // Collider (Adjusted for scale)
    // Reduced width to 20 and height to 80, added Y offset 10 to lower it slightly
    mCollider = new AABBColliderComponent(this, 0, 10, 20, 80, ColliderLayer::Enemy);
    mCollider->SetEnabled(false); // Disable collider during warning
}

void CactusPillar::OnUpdate(float deltaTime)
{
    if (mIsWarning) {
        mWarningTimer -= deltaTime;
        
        // Blink effect using the cactus sprite itself
        // We want to show it briefly at the target position (ground) or just blink it where it is?
        // If it is at y+100, it is below ground. If we blink it there, it might not be visible if there is ground.
        // But in this game, actors are usually in front.
        // Let's move it to target Y for the blink, then move back? No that's jerky.
        // Let's just blink it at the spawn position (below ground) but maybe the user wants to see it AT the ground.
        // "make the cactus come from the ground as they were sprouting so the player knows where they come from"
        // If I show it at ground level blinking, then it disappears and rises from below? That's weird.
        // Maybe I should just make it rise slowly from the start?
        // Or maybe show a "ghost" or transparent version at the target location?
        // The user said "remove the block, keep only the cactus visible".
        // Let's try this: During warning, the cactus is at the target Y (ground level) but blinking/transparent.
        // Then when warning ends, it snaps to bottom and rises? Or just stays there?
        // "make them rise form the platform" implies movement.
        // So: Warning -> Blink at ground level (to show WHERE). Then -> Snap to bottom -> Rise.
        
        if (static_cast<int>(mWarningTimer * 10) % 2 == 0) {
            mSprite->SetVisible(true);
            mSprite->SetAlpha(0.5f); // 50% opacity
            // Temporarily set position to target Y for the blink visual
            Vector2 currentPos = GetPosition();
            mSprite->SetDrawOffset(Vector2(0.0f, mTargetY - currentPos.y)); 
        } else {
            mSprite->SetVisible(false);
        }

        if (mWarningTimer <= 0.0f) {
            mIsWarning = false;
            mSprite->SetVisible(true);
            mSprite->SetAlpha(1.0f); // Full opacity
            mSprite->SetDrawOffset(Vector2::Zero); // Reset offset
            mCollider->SetEnabled(true);
        }
        return; // Don't rise yet
    }

    mLifeTime += deltaTime;

    if (mRising) {
        Vector2 pos = GetPosition();
        pos.y -= mRiseSpeed * deltaTime;
        if (pos.y <= mTargetY) {
            pos.y = mTargetY;
            mRising = false;
        }
        SetPosition(pos);
    } else {
        if (mLifeTime > 4.0f) { // Last a bit longer
            SetState(ActorState::Destroy);
        }
    }
}

void CactusPillar::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Player) {
        // Damage player
        Spaceman* player = dynamic_cast<Spaceman*>(other->GetOwner());
        if (player) {
            GetGame()->SetGameOverInfo(this);
            player->Kill();
        }
    }
}

void CactusPillar::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (other->GetLayer() == ColliderLayer::Player) {
        // Damage player
        Spaceman* player = dynamic_cast<Spaceman*>(other->GetOwner());
        if (player) {
            GetGame()->SetGameOverInfo(this);
            player->Kill();
        }
    }
}
