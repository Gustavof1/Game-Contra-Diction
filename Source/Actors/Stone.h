#pragma once
#include "Actor.h"

class Stone : public Actor
{
public:
    Stone(class Game* game);
    void Kill() override;
};
