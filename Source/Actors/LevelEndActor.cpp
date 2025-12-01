#include "LevelEndActor.h"
#include "../Game.h"
#include "../Components/Drawing/SpriteComponent.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/Font.h"

// --- CLASSE INTERNA PARA DESENHAR O TEXTO ---
class EndTextComponent : public DrawComponent
{
public:
    EndTextComponent(LevelEndActor* owner)
        : DrawComponent(owner, 250) // DrawOrder 250 (bem na frente de tudo)
        , mOwnerActor(owner)
    {
    }

    void Draw(Renderer* renderer) override
    {
        // Só desenha se o timer do ator for maior que 1.0s
        if (mOwnerActor->GetTimer() > 1.0f)
        {
            Font* font = renderer->GetFont("../Assets/Fonts/ALS_Micro_Bold.ttf");
            if (font) {
                Texture* textTex = font->RenderText("NIVEL CONCLUIDO!", Vector3(0.0f, 1.0f, 0.0f), 48);

                if (textTex) {
                    Vector2 camera = mOwnerActor->GetGame()->GetCameraPos();
                    // Centraliza no topo da tela
                    Vector2 textPos = camera + Vector2(Game::WINDOW_WIDTH/2.0f - textTex->GetWidth()/2.0f, 100.0f);

                    // Usando a nova assinatura do DrawTexture (com Scale Vector2)
                    renderer->DrawTexture(
                        textPos,
                        Vector2(static_cast<float>(textTex->GetWidth()), static_cast<float>(textTex->GetHeight())),
                        0.0f,
                        Vector3::One,
                        textTex,
                        Vector4::UnitRect,
                        camera,
                        Vector2(1.0f, 1.0f) // Scale normal (sem flip)
                    );

                    delete textTex; // Limpa textura temporária
                }
            }
        }
    }

private:
    LevelEndActor* mOwnerActor;
};

LevelEndActor::LevelEndActor(Game* game, const std::string& shipTexture)
    :Actor(game)
    ,mTimer(0.0f)
    ,mIsAnimating(true)
{
    mSprite = new SpriteComponent(this, 200); // Draw order bem alto pra ficar na frente
    auto* tex = game->GetRenderer()->GetTexture(shipTexture);
    mSprite->SetTexture(tex);

    new EndTextComponent(this);

    // Começa pequeno
    SetScale(Vector2(1.0f));
}

void LevelEndActor::OnUpdate(float deltaTime)
{
    mTimer += deltaTime;

    // 1. Animação de Crescimento e Centralização (0 a 1.5s)
    if (mTimer < 1.5f) {
        float t = mTimer / 1.5f;
        // Lerp scale
        SetScale(Vector2(1.0f + t * 2.0f)); // Cresce até 3x
        
        // Centralizar na câmera
        Vector2 camera = GetGame()->GetCameraPos();
        Vector2 screenCenter = camera + Vector2(Game::WINDOW_WIDTH/2, Game::WINDOW_HEIGHT/2);
        
        // Move suavemente da posição inicial para o centro
        Vector2 currentPos = GetPosition();
        Vector2 newPos = currentPos + (screenCenter - currentPos) * (deltaTime * 5.0f);
        SetPosition(newPos);
    }
    
    // 2. Fim do Timer (4.0s) -> Próxima Fase
    if (mTimer > 4.0f) {
        // Lógica para trocar de fase
        GameScene current = GetGame()->GetCurrentScene();
        if (current == GameScene::Level1) {
            GetGame()->SetScene(GameScene::Level2);
        } else if (current == GameScene::Level2) {
            GetGame()->SetScene(GameScene::Level3);
        } else {
            // Fim do jogo
            GetGame()->SetScene(GameScene::MainMenu);
        }
    }
}