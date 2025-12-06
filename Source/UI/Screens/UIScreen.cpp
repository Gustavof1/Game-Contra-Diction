// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "UIScreen.h"
#include "../../Game.h"
#include "../../Renderer/Shader.h"

UIScreen::UIScreen(Game* game, const std::string& fontName)
	:mGame(game)
	,mPos(0.f, 0.f)
	,mSize(0.f, 0.f)
	,mState(UIState::Active)
    ,mSelectedButtonIndex(-1)
{
    mGame->PushUI(this);
    mFont = mGame->GetRenderer()->GetFont(fontName);
}

UIScreen::~UIScreen()
{
    for (auto t : mTexts) {
        delete t;
    }
    mTexts.clear();

    for (auto b : mButtons) {
        delete b;
    }
	mButtons.clear();

    for (auto img : mImages) {
        delete img;
    }
    mImages.clear();

    for (auto rect : mRects) {
        delete rect;
    }
    mRects.clear();
}

void UIScreen::Update(float deltaTime)
{
	
}

void UIScreen::HandleKeyPress(int key)
{
    if (mButtons.empty()) return;

    int previousIndex = mSelectedButtonIndex;

    if (key == SDLK_UP || key == SDLK_w)
    {
        mSelectedButtonIndex--;
        if (mSelectedButtonIndex < 0)
        {
            mSelectedButtonIndex = static_cast<int>(mButtons.size()) - 1;
        }
    }
    else if (key == SDLK_DOWN || key == SDLK_s)
    {
        mSelectedButtonIndex++;
        if (mSelectedButtonIndex >= static_cast<int>(mButtons.size()))
        {
            mSelectedButtonIndex = 0;
        }
    }
    else if (key == SDLK_RETURN || key == SDLK_SPACE || key == SDLK_KP_ENTER)
    {
        if (mSelectedButtonIndex >= 0 && mSelectedButtonIndex < static_cast<int>(mButtons.size()))
        {
            mButtons[mSelectedButtonIndex]->OnClick();
        }
    }

    if (mSelectedButtonIndex != previousIndex)
    {
        for (int i = 0; i < static_cast<int>(mButtons.size()); i++)
        {
            mButtons[i]->SetHighlighted(i == mSelectedButtonIndex);
        }
        mGame->GetAudio()->PlaySound("Bump.wav");
    }
}

void UIScreen::HandleMouseMove(const Vector2& mousePos)
{
    if (mButtons.empty()) return;

    int previousIndex = mSelectedButtonIndex;
    bool anyHovered = false;

    for (int i = 0; i < static_cast<int>(mButtons.size()); i++)
    {
        if (mButtons[i]->ContainsPoint(mousePos))
        {
            mSelectedButtonIndex = i;
            anyHovered = true;
            break;
        }
    }

    if (anyHovered && mSelectedButtonIndex != previousIndex)
    {
        for (int i = 0; i < static_cast<int>(mButtons.size()); i++)
        {
            mButtons[i]->SetHighlighted(i == mSelectedButtonIndex);
        }
        mGame->GetAudio()->PlaySound("Bump.wav");
    }
}

void UIScreen::HandleMouseClick(const Vector2& mousePos)
{
    for (auto* b : mButtons)
    {
        if (b->ContainsPoint(mousePos))
        {
            b->OnClick();
            break;
        }
    }
}

void UIScreen::Close()
{
	mState = UIState::Closing;
}

UIText* UIScreen::AddText(const std::string& name, const Vector2& offset, float scale, float angle, const int pointSize, const int unsigned wrapLength, int drawOrder)
{
    auto* text = new UIText(mGame, name, mFont, offset, scale, angle, pointSize, wrapLength, drawOrder);
    mTexts.emplace_back(text);
    return text;
}

UIButton* UIScreen::AddButton(const std::string& name, std::function<void()> onClick, const Vector2& offset, float scale, float angle, const int pointSize, const int unsigned wrapLength, int drawOrder)
{
    auto* button = new UIButton(mGame, onClick, name, mFont, offset, scale, angle, pointSize, wrapLength, drawOrder);
    mButtons.emplace_back(button);

    if (mButtons.size() == 1) {
        mSelectedButtonIndex = 0;
        button->SetHighlighted(true);
    }

    return button;
}

UIImage* UIScreen::AddImage(const std::string& imagePath, const Vector2& offset, float scale, float angle, int drawOrder)
{
    auto* image = new UIImage(mGame, imagePath, offset, scale, angle, drawOrder);
    mImages.emplace_back(image);
    return image;
}

UIRect *UIScreen::AddRect(const Vector2 &offset, const Vector2 &size, float scale, float angle, int drawOrder)
{
    auto* rect = new UIRect(mGame, offset, size, scale, angle, drawOrder);
    mRects.emplace_back(rect);
    return rect;
}