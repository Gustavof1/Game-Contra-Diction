#include "Grass.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Game.h"
#include "../Random.h"
#include <string>

Grass::Grass(Game* game)
    :Actor(game)
{
    SpriteComponent* sc = new SpriteComponent(this, 150);
    
    // Randomly select a vegetation type
    // 0: Bush, 1: Flower, 2: Grass, 3: Mushroom
    int type = Random::GetIntRange(0, 3);
    std::string folder;
    std::string prefix;
    int maxIndex = 1;

    switch (type)
    {
    case 0: // Bush
        folder = "Bush";
        prefix = "Bush-";
        maxIndex = 2;
        break;
    case 1: // Flower
        folder = "Flower";
        prefix = "Flower-";
        maxIndex = 15;
        break;
    case 2: // Grass
        folder = "Grass";
        prefix = "Grass-";
        maxIndex = 4;
        break;
    case 3: // Mushroom
        folder = "Mushroom";
        prefix = "Mushroom-";
        maxIndex = 12;
        break;
    }

    int index = Random::GetIntRange(1, maxIndex);
    std::string path = "../Assets/Sprites/ObjectsScenery-ContraDiction/" + folder + "/" + prefix + std::to_string(index) + ".png";

    sc->SetTexture(game->GetRenderer()->GetTexture(path));
    sc->SetIsVegetation(true);

    // Collider for destruction
    // Assuming 32x32 roughly
    // Set as trigger so player doesn't collide with it physically
    AABBColliderComponent* cc = new AABBColliderComponent(this, 0, 0, 32, 32, ColliderLayer::Destructible, true);
}

void Grass::Kill()
{
    GetGame()->GetAudio()->PlaySound("Kick.wav");
    SetState(ActorState::Destroy);
}
