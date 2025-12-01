#include "Stone.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Game.h"
#include "../Random.h"
#include <string>

Stone::Stone(Game* game)
    :Actor(game)
{
    SpriteComponent* sc = new SpriteComponent(this, 150);
    
    int rockNum = Random::GetIntRange(1, 3);
    std::string path = "../Assets/Sprites/ObjectsScenery-ContraDiction/Rock/Rock-" + std::to_string(rockNum) + ".png";
    
    sc->SetTexture(game->GetRenderer()->GetTexture(path));

    // Assuming rocks are roughly 32x32
    // Set as trigger so player doesn't collide with it physically
    AABBColliderComponent* cc = new AABBColliderComponent(this, 0, 0, 32, 32, ColliderLayer::Destructible, true);
}

void Stone::Kill()
{
    GetGame()->GetAudio()->PlaySound("Kick.wav");
    SetState(ActorState::Destroy);
}
