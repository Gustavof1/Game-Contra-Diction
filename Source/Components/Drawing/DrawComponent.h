//
// Created by Lucas N. Ferreira on 03/08/23.
//

#pragma once
#include "../Component.h"
#include "../../Math.h"
#include "../../Renderer/Renderer.h"
#include "../../Renderer/VertexArray.h"
#include <vector>
#include <SDL.h>

class DrawComponent : public Component
{
public:
    // (Lower draw order corresponds with further back)
    DrawComponent(class Actor* owner, int drawOrder = 100);
    ~DrawComponent();

    virtual void Draw(Renderer* renderer);
    int GetDrawOrder() const { return mDrawOrder; }

    bool IsVisible() const { return mIsVisible; }
    void SetVisible(bool visible) { mIsVisible = visible; }
    void SetColor(const Vector3& color) { mColor = color; }
    
    void SetIsVegetation(bool isVegetation) { mIsVegetation = isVegetation; }
    bool IsVegetation() const { return mIsVegetation; }

protected:
    int mDrawOrder;
    bool mIsVisible;
    bool mIsVegetation;
    Vector3 mColor;
};
