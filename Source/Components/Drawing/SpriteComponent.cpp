#include "SpriteComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include "../../Renderer/Renderer.h"

SpriteComponent::SpriteComponent(Actor* owner, int drawOrder)
    :DrawComponent(owner, drawOrder)
    ,mTexture(nullptr)
    ,mTexWidth(0)
    ,mTexHeight(0)
    ,mUseSrcRect(false) // Por padrão, desenha a imagem toda
    ,mDrawOffset(Vector2::Zero)
{
}

SpriteComponent::~SpriteComponent()
{
}

void SpriteComponent::Draw(Renderer* renderer)
{
    if (mTexture)
    {
        Vector2 scale = mOwner->GetScale();

        // Ajuste para usar o recorte ou tamanho total
        int width = mUseSrcRect ? mSrcRect.w : mTexWidth;
        int height = mUseSrcRect ? mSrcRect.h : mTexHeight;

        Vector2 size(
            static_cast<float>(width) * scale.x,
            static_cast<float>(height) * scale.y
        );

        Vector2 pos = mOwner->GetPosition();
        pos += mDrawOffset; // Apply offset
        float rot = mOwner->GetRotation();

        // Lógica do Recorte (UVs)
        Vector4 srcRect = Vector4::UnitRect;
        if (mUseSrcRect) {
            float invW = 1.0f / mTexture->GetWidth();
            float invH = 1.0f / mTexture->GetHeight();
            srcRect.x = mSrcRect.x * invW;
            srcRect.y = mSrcRect.y * invH;
            srcRect.z = mSrcRect.w * invW;
            srcRect.w = mSrcRect.h * invH;
        }

        Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();

        renderer->DrawTexture(
            pos,
            size,
            rot,
            mColor,
            mTexture,
            srcRect,
            cameraPos,
            mFlipScale,
            1.0f,
            1.0f,
            mIsVegetation);
    }
}

void SpriteComponent::SetTexture(Texture* texture)
{
    mTexture = texture;
    // Pega o tamanho original da imagem
    mTexWidth = texture->GetWidth();
    mTexHeight = texture->GetHeight();
}

void SpriteComponent::SetTextureRect(int x, int y, int w, int h)
{
    mSrcRect.x = x;
    mSrcRect.y = y;
    mSrcRect.w = w;
    mSrcRect.h = h;
    mUseSrcRect = true;
}