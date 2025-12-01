#pragma once
#include "DrawComponent.h"
#include "SDL.h"
#include <string>

class SpriteComponent : public DrawComponent
{
public:
    // O drawOrder define quem é desenhado na frente (número maior = mais na frente)
    SpriteComponent(class Actor* owner, int drawOrder = 100);
    ~SpriteComponent();

    void Draw(class Renderer* renderer) override;
    virtual void SetTexture(class Texture* texture);

    // --- A MÁGICA DO TILESET ---
    // Esta função define qual pedaço da textura desenhar
    void SetTextureRect(int x, int y, int w, int h);
    
    void SetDrawOffset(const Vector2& offset) { mDrawOffset = offset; }

    int GetTexHeight() const { return mTexHeight; }
    int GetTexWidth() const { return mTexWidth; }

    void SetFlipScale(const Vector2& scale) { mFlipScale = scale; }
    
    void SetColor(const Vector3& color) { mColor = color; }

protected:
    class Texture* mTexture;
    Vector2 mDrawOffset;
    int mTexWidth;
    int mTexHeight;
    Vector3 mColor { 1.0f, 1.0f, 1.0f };

    // Retângulo de recorte (Source Rect)
    SDL_Rect mSrcRect; 
    // Flag para saber se usamos o recorte ou a imagem toda
    bool mUseSrcRect;
    Vector2 mFlipScale { 1.0f, 1.0f };
};