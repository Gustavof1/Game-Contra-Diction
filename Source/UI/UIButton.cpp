#include "UIButton.h"
#include "../Renderer/Texture.h"
#include "../Renderer/Shader.h" 
#include "../Game.h"

UIButton::UIButton(class Game* game, std::function<void()> onClick, const std::string& text, class Font* font,
                   const Vector2 &offset, float scale, float angle, int pointSize, const unsigned wrapLength, int drawOrder)
        :UIText(game, text, font, offset, scale, angle, pointSize, wrapLength, drawOrder)
        ,mOnClick(onClick)
        ,mHighlighted(false)
        ,mNormalTexture(nullptr)
        ,mHoverTexture(nullptr)
        ,mPressedTexture(nullptr)
{

}

UIButton::~UIButton()
{

}

void UIButton::SetTextures(const std::string& normal, const std::string& hover, const std::string& pressed)
{
    auto renderer = mGame->GetRenderer();
    mNormalTexture = renderer->GetTexture(normal);
    if (!hover.empty())
    {
        mHoverTexture = renderer->GetTexture(hover);
    }
    if (!pressed.empty())
    {
        mPressedTexture = renderer->GetTexture(pressed);
    }
}

void UIButton::OnClick()
{
    if (mOnClick) {
        mOnClick();
    }
}

bool UIButton::ContainsPoint(const Vector2& pt) const
{
    float width = 0.0f;
    float height = 0.0f;

    if (mNormalTexture)
    {
        width = static_cast<float>(mNormalTexture->GetWidth()) * mScale;
        height = static_cast<float>(mNormalTexture->GetHeight()) * mScale;
    }
    else if (mTexture)
    {
        width = (static_cast<float>(mTexture->GetWidth()) + mMargin.x) * mScale;
        height = (static_cast<float>(mTexture->GetHeight()) + mMargin.y) * mScale;
    }
    else
    {
        return false;
    }

    bool noX = pt.x < (mOffset.x - width / 2.0f) || pt.x > (mOffset.x + width / 2.0f);
    bool noY = pt.y < (mOffset.y - height / 2.0f) || pt.y > (mOffset.y + height / 2.0f);

    return !(noX || noY);
}

void UIButton::Draw(class Shader* shader)
{
    if (!mIsVisible) return;

    if (mNormalTexture)
    {
        Texture* tex = mNormalTexture;
        if (mHighlighted && mHoverTexture)
        {
            tex = mHoverTexture;
        }
        // We could handle pressed state here if we tracked it

        int width = tex->GetWidth();
        int height = tex->GetHeight();

        Matrix4 scaleMat = Matrix4::CreateScale(static_cast<float>(width) * mScale,
                                                static_cast<float>(height) * mScale,
                                                1.0f);
        Matrix4 transMat = Matrix4::CreateTranslation(Vector3(mOffset.x, mOffset.y, 0.0f));
        Matrix4 world = scaleMat * transMat;
        shader->SetMatrixUniform("uWorldTransform", world);
        shader->SetFloatUniform("uTextureFactor", 1.0f); // Use texture
        shader->SetVectorUniform("uColor", Vector4(1.0f, 1.0f, 1.0f, 1.0f)); // White tint
        shader->SetVectorUniform("uTexRect", Vector4(0.0f, 0.0f, 1.0f, 1.0f)); // Reset texture rect

        tex->SetActive();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        
        // Draw text on top if it exists
        UIText::Draw(shader);
    }
    else
    {
        // 1. Configura a transparência baseada no destaque (Highlight)
        Vector4 colorToDraw = mBackgroundColor;
        if(!mHighlighted) {
            colorToDraw.w *= 0.6f; // Mais transparente se não selecionado
        }

        // 2. DESENHAR O FUNDO (RETÂNGULO)
        // Precisamos da textura do texto para saber o tamanho do botão
        if (mTexture) 
        {
            float width = static_cast<float>(mTexture->GetWidth());
            float height = static_cast<float>(mTexture->GetHeight());

            // Calcula escala do retângulo (Tamanho do Texto + Margem)
            Matrix4 scaleMat = Matrix4::CreateScale((width + mMargin.x) * mScale,
                                                    (height + mMargin.y) * mScale, 1.0f);

            // Translada para a posição correta
            Matrix4 transMat = Matrix4::CreateTranslation(Vector3(mOffset.x, mOffset.y, 0.0f));

            Matrix4 world = scaleMat * transMat;
            shader->SetMatrixUniform("uWorldTransform", world);

            // Configura shader para desenhar COR SÓLIDA (sem textura)
            shader->SetFloatUniform("uTextureFactor", 0.0f);
            
            // --- CORREÇÃO DO NOME ---
            // Aqui enviamos a cor de fundo do botão.
            // O nome DEVE ser "uColor" para bater com o Base.frag que ajustamos.
            shader->SetVectorUniform("uColor", colorToDraw); 
            // ------------------------

            // Desenha o quadrado
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }

        // 3. DESENHAR O TEXTO
        // Chama a classe pai (UIText) para desenhar as letras por cima do quadrado
        
        // Ajuste para centralizar o texto verticalmente (compensa descenders da fonte)
        float oldY = mOffset.y;
        mOffset.y += 7.0f;
        UIText::Draw(shader);
        mOffset.y = oldY;
    }
}