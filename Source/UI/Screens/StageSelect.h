#pragma once
#include "UIScreen.h"

class StageSelect : public UIScreen
{
public:
    StageSelect(class Game* game, const std::string& fontName);
    ~StageSelect();

    void HandleKeyPress(int key) override;
};
