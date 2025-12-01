#pragma once
#include "Actor.h"

class Grass : public Actor
{
public:
    Grass(class Game* game);
    void Kill() override;
};
