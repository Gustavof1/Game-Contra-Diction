#include "VineWarning.h"
#include "Vine.h"
#include "../Game.h"
#include "../Components/Drawing/SpriteComponent.h"

VineWarning::VineWarning(Game* game)
    :Actor(game)
    ,mLifeTime(1.0f)
{
    SpriteComponent* sc = new SpriteComponent(this);
    sc->SetTexture(game->GetRenderer()->GetTexture("../Assets/Sprites/Blocks/BlockA.png"));
    sc->SetColor(Vector3(1.0f, 1.0f, 0.0f)); // Yellow
}

void VineWarning::OnUpdate(float deltaTime)
{
    mLifeTime -= deltaTime;
    if (mLifeTime <= 0.0f) {
        // Spawn Vine
        auto* vine = new Vine(GetGame());
        vine->SetPosition(GetPosition());
        
        SetState(ActorState::Destroy);
    }
}
