#include "HealthBarComponent.h"
#include "../Actors/Actor.h"
#include "../Renderer/Renderer.h"
#include "../Game.h"

HealthBarComponent::HealthBarComponent(Actor* owner, int maxHP, int* currentHP)
    : DrawComponent(owner)
    , mMaxHP(maxHP)
    , mCurrentHP(currentHP)
{
}

void HealthBarComponent::Draw(Renderer* renderer)
{
    if (!mCurrentHP) return;

    Vector2 pos = mOwner->GetPosition();
    pos.y -= 100.0f; // Above head
    
    float healthPct = static_cast<float>(*mCurrentHP) / static_cast<float>(mMaxHP);
    healthPct = Math::Clamp(healthPct, 0.0f, 1.0f);
    
    Vector2 barSize(100.0f, 10.0f);
    
    // Draw background (red)
    renderer->DrawRect(pos, barSize, 0.0f, Vector3(1.0f, 0.0f, 0.0f), mOwner->GetGame()->GetCameraPos(), RendererMode::TRIANGLES);
    
    // Draw foreground (green)
    Vector2 currentSize(barSize.x * healthPct, barSize.y);
    // Adjust position to be left-aligned relative to the center
    Vector2 currentPos = pos;
    currentPos.x -= (barSize.x - currentSize.x) / 2.0f;
    
    renderer->DrawRect(currentPos, currentSize, 0.0f, Vector3(0.0f, 1.0f, 0.0f), mOwner->GetGame()->GetCameraPos(), RendererMode::TRIANGLES);
}
