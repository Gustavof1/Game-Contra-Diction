#pragma once
#include "../../../Source/Actors/Actor.h"

class Vegetation : public Actor
{
public:
    Vegetation(class Game* game);

    // Função helper para configurar o visual de uma vez
    void SetSpriteData(class Texture* texture, int srcX, int srcY, int w, int h);
};
