//
// Created by Lucas N. Ferreira on 22/05/25.
//

#include "UIText.h"
#include "../Renderer/Font.h"
#include "../Renderer/Texture.h"
#include "../Renderer/Shader.h"

UIText::UIText(class Game* game, const std::string& text, class Font* font, const Vector2 &offset, float scale, float angle,
               int pointSize, const unsigned wrapLength, int drawOrder)
   :UIImage(game, offset, scale, angle, drawOrder)
   ,mFont(font)
   ,mPointSize(pointSize)
   ,mWrapLength(wrapLength)
   ,mTextColor(Color::White)
   ,mBackgroundColor(0.0f,0.0f,0.0f,1.0f)
   ,mMargin(Vector2(50.0f, 10.f))
{
    SetText(text);
}

UIText::~UIText()
{

}

void UIText::SetText(const std::string &text)
{
    // Clear out previous title texture if it exists
    if (mTexture)
    {
        mTexture->Unload();
        delete mTexture;
        mTexture = nullptr;
    }

    // Create texture for title
    mText = text;
    mTexture = mFont->RenderText(mText, mTextColor, mPointSize, mWrapLength);

    if (mTexture) {
        mSize = Vector2(static_cast<float>(mTexture->GetWidth()), 
                        static_cast<float>(mTexture->GetHeight()));
    }
    else {
        mSize = Vector2::Zero;
    }
}

void UIText::SetTextColor(const Vector3 &color)
{
    // Clear out previous title texture if it exists
    if (mTexture)
    {
        mTexture->Unload();
        delete mTexture;
        mTexture = nullptr;
    }

    mTextColor = color;
    mTexture = mFont->RenderText(mText, mTextColor, mPointSize, mWrapLength);

    if (mTexture) {
        mSize = Vector2(static_cast<float>(mTexture->GetWidth()), 
                        static_cast<float>(mTexture->GetHeight()));
    }
    else {
        mSize = Vector2::Zero;
    }
}

void UIText::Draw(class Shader* shader)
{
    if(!mTexture || !mIsVisible)
        return;

    // Draw text
    UIImage::Draw(shader);
}