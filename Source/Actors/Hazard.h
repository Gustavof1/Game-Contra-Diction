#pragma once
#include "Actor.h"

class Hazard : public Actor
{
public:
    // Agora aceitamos width e height dinâmicos
    Hazard(class Game* game, int width, int height);
};