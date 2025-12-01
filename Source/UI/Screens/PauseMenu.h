//
// Created by Vinicius Trindade on 12/11/25.
//

#pragma once

#include "UIScreen.h"
#include "../../Game.h"

class PauseMenu : public UIScreen
{
public:
    PauseMenu(class Game* game, const std::string& fontName, GameScene level);
    ~PauseMenu();

    void HandleKeyPress(int key) override;

    
private:
    GameScene mlevel;
    bool mResuming;
};