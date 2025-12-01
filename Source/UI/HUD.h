#pragma once
#include "../Math.h"

class HUD {
public:
    HUD(class Game* game);
    ~HUD();

    void Draw(class Renderer* renderer);

private:
    class Game* mGame;
    class Texture* mTexPistol;
    class Texture* mTexAlienPistol;
    class Texture* mTexFlashlight;
    class Texture* mTexHeadphones;
};
