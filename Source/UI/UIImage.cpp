//
// Created by Lucas N. Ferreira on 28/05/25.
//

#include "UIImage.h"
#include "../Renderer/Texture.h"
#include "../Renderer/Shader.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexArray.h"
#include "../Actors/Actor.h"
#include "../Game.h"


UIImage::UIImage(class Game* game, const Vector2 &offset, const float scale, const float angle, int drawOrder)
        :UIElement(game, offset, scale, angle, drawOrder)
        ,mTexture(nullptr)
        ,mSize(Vector2::Zero)
        ,mColor(1.0f, 1.0f, 1.0f, 1.0f)
        ,mUseSrcRect(false)
{

}

UIImage::UIImage(class Game* game, const std::string &imagePath, const Vector2 &offset, const float scale, const float angle, int drawOrder)
        :UIElement(game, offset, scale, angle)
        ,mSize(Vector2::Zero)
        ,mColor(1.0f, 1.0f, 1.0f, 1.0f)
        ,mUseSrcRect(false)
{
    mTexture = GetGame()->GetRenderer()->GetTexture(imagePath);
    if(mTexture)
    {
        mSize = Vector2(static_cast<float>(mTexture->GetWidth()), 
                        static_cast<float>(mTexture->GetHeight()));
    }
}

UIImage::~UIImage()
{

}

void UIImage::Draw(class Shader* shader)
{
    if(!mTexture || !mIsVisible)
        return;

    Matrix4 scaleMat = Matrix4::CreateScale(mSize.x * mScale, mSize.y * mScale, 1.0f);

    Matrix4 rotMat = Matrix4::CreateRotationZ(mAngle);
    Matrix4 transMat = Matrix4::CreateTranslation(Vector3(mOffset.x, mOffset.y, 0.0f));

    Matrix4 world = scaleMat * rotMat * transMat;
    shader->SetMatrixUniform("uWorldTransform", world);

    shader->SetFloatUniform("uTextureFactor", 1.0f);
    shader->SetVectorUniform("uColor", mColor);

    Vector4 texRect(0.0f, 0.0f, 1.0f, 1.0f);
    if (mUseSrcRect && mTexture)
    {
        float invW = 1.0f / mTexture->GetWidth();
        float invH = 1.0f / mTexture->GetHeight();
        texRect.x = mSrcRect.x * invW;
        texRect.y = mSrcRect.y * invH;
        texRect.z = mSrcRect.w * invW;
        texRect.w = mSrcRect.h * invH;
    }
    shader->SetVectorUniform("uTexRect", texRect);
    
    mTexture->SetActive();

    auto* verts = GetGame()->GetRenderer()->GetSpriteVerts();
    verts->SetActive();

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void UIImage::SetTextureRect(int x, int y, int w, int h)
{
    mSrcRect.x = x;
    mSrcRect.y = y;
    mSrcRect.w = w;
    mSrcRect.h = h;
    mUseSrcRect = true;
    mSize = Vector2(static_cast<float>(w), static_cast<float>(h));
}
