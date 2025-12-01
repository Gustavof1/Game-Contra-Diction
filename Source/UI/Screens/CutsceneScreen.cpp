#include "CutsceneScreen.h"
#include "../../Game.h"
#include <SDL.h>

CutsceneScreen::CutsceneScreen(Game* game, GameScene nextScene)
    : UIScreen(game, "../Assets/Fonts/ALS_Micro_Bold.ttf")
    , mState(CutsceneState::Frame1)
    , mNextScene(nextScene)
    , mTimer(0.0f)
    , mFadeTimer(0.0f)
{
    // Setup images
    // "middle top" - Center X, Top Y (maybe 1/3 down?)
    Vector2 topCenter(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT / 3.0f);

    // Background (Black)
    auto bg = AddRect(Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT/2.0f), Vector2(Game::WINDOW_WIDTH, Game::WINDOW_HEIGHT), 1.0f, 0.0f, 0);
    bg->SetColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f));

    // Load images
    // Frame 1 shrunken (scale 0.5f)
    mImage1 = AddImage("../Assets/Cutscenes/start/frame1.PNG", topCenter, 0.5f);
    
    // Frame 2
    mImage2 = AddImage("../Assets/Cutscenes/start/frame2.PNG", topCenter, 0.5f);
    mImage2->SetIsVisible(false);

    // Frame 3
    mImage3 = AddImage("../Assets/Cutscenes/start/frame3.PNG", topCenter, 0.5f);
    mImage3->SetIsVisible(false);

    // Setup Text
    // "write below it"
    Vector2 textPos(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT * 0.85f);
    mText = AddText("Ele estava distraido no jogo do tigrinho", textPos, 0.7f);
    // Assuming AddText centers the text or we rely on default alignment.
    // If UIText doesn't center, we might need to adjust.
    // But usually UI systems in these books center text if position is center.
    // Let's assume it does for now.
    
    // Setup Fade Rect
    // Full screen black rect
    mFadeRect = AddRect(Vector2(Game::WINDOW_WIDTH/2.0f, Game::WINDOW_HEIGHT/2.0f), Vector2(Game::WINDOW_WIDTH, Game::WINDOW_HEIGHT));
    mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent initially
    // Note: AddRect usually takes center position.
}

CutsceneScreen::~CutsceneScreen()
{
}

void CutsceneScreen::Update(float deltaTime)
{
    UIScreen::Update(deltaTime);
    
    mTimer += deltaTime;

    switch (mState)
    {
        case CutsceneState::Frame1:
            if (mTimer >= DISPLAY_DURATION)
            {
                mState = CutsceneState::FadeOut1;
                mTimer = 0.0f;
                mFadeTimer = 0.0f;
            }
            break;
            
        case CutsceneState::FadeOut1:
            mFadeTimer += deltaTime;
            if (mFadeTimer >= FADE_DURATION)
            {
                mFadeTimer = 0.0f;
                mState = CutsceneState::FadeIn2;
                
                // Switch content
                mImage1->SetIsVisible(false);
                mImage2->SetIsVisible(true);
                mText->SetText("E nao percebeu a nave quebrando");
            }
            else
            {
                float alpha = mFadeTimer / FADE_DURATION;
                mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, alpha));
            }
            break;

        case CutsceneState::FadeIn2:
            mFadeTimer += deltaTime;
            if (mFadeTimer >= FADE_DURATION)
            {
                mFadeTimer = 0.0f;
                mState = CutsceneState::Frame2;
                mTimer = 0.0f;
                mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
            }
            else
            {
                float alpha = 1.0f - (mFadeTimer / FADE_DURATION);
                mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, alpha));
            }
            break;

        case CutsceneState::Frame2:
            if (mTimer >= DISPLAY_DURATION)
            {
                mState = CutsceneState::FadeOut2;
                mTimer = 0.0f;
                mFadeTimer = 0.0f;
            }
            break;

        case CutsceneState::FadeOut2:
            mFadeTimer += deltaTime;
            if (mFadeTimer >= FADE_DURATION)
            {
                mFadeTimer = 0.0f;
                mState = CutsceneState::FadeIn3;
                
                // Switch content
                mImage2->SetIsVisible(false);
                mImage3->SetIsVisible(true);
                mText->SetText("Ele precisou ejetar e sua nave caiu, agora ele precisa encontrar a os destrocos para pedir resgate e escapar do planeta desconhecido");
            }
            else
            {
                float alpha = mFadeTimer / FADE_DURATION;
                mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, alpha));
            }
            break;

        case CutsceneState::FadeIn3:
            mFadeTimer += deltaTime;
            if (mFadeTimer >= FADE_DURATION)
            {
                mFadeTimer = 0.0f;
                mState = CutsceneState::Frame3;
                mTimer = 0.0f;
                mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
            }
            else
            {
                float alpha = 1.0f - (mFadeTimer / FADE_DURATION);
                mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, alpha));
            }
            break;

        case CutsceneState::Frame3:
            if (mTimer >= DISPLAY_DURATION)
            {
                mState = CutsceneState::FadeOut3;
                mTimer = 0.0f;
                mFadeTimer = 0.0f;
            }
            break;

        case CutsceneState::FadeOut3:
            mFadeTimer += deltaTime;
            if (mFadeTimer >= FADE_DURATION)
            {
                mState = CutsceneState::Finished;
                Close();
                mGame->SetScene(mNextScene);
            }
            else
            {
                float alpha = mFadeTimer / FADE_DURATION;
                mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, alpha));
            }
            break;
            
        case CutsceneState::Finished:
            break;
    }
}

void CutsceneScreen::HandleKeyPress(int key)
{
    // Skip cutscene on any key press
    Close();
    mGame->SetScene(mNextScene);
}
