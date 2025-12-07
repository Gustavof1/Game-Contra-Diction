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

    std::string text1;

    if (mNextScene == GameScene::Level2) {
        // Transition 1 -> 2
        mImage1 = AddImage("../Assets/Cutscenes/transition 1 to 2/1.PNG", topCenter, 0.5f);
        mImage2 = AddImage("../Assets/Cutscenes/transition 1 to 2/2.PNG", topCenter, 0.5f);
        mImage3 = AddImage("../Assets/Cutscenes/transition 1 to 2/3.PNG", topCenter, 0.5f);
        text1 = "Ele encontrou um carro, deve pertencer a familia que ele encontrou andando na floresta";
    } else {
        // Default / Intro
        mImage1 = AddImage("../Assets/Cutscenes/start/frame1.PNG", topCenter, 0.5f);
        mImage2 = AddImage("../Assets/Cutscenes/start/frame2.PNG", topCenter, 0.5f);
        mImage3 = AddImage("../Assets/Cutscenes/start/frame3.PNG", topCenter, 0.5f);
        text1 = "Ele estava distraido no jogo do tigrinho";
    }
    
    mImage2->SetIsVisible(false);
    mImage3->SetIsVisible(false);

    // Setup Text
    // "write below it"
    Vector2 textPos(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT * 0.85f);
    mText = AddText(text1, textPos, 0.7f);
    
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
                if (mNextScene == GameScene::Level2) {
                    mText->SetText("Ele nao pensou 2 vezes e ja roubou");
                } else {
                    mText->SetText("E nao percebeu a nave quebrando");
                }
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
                if (mNextScene == GameScene::Level2) {
                    mText->SetText("Ele chegou em uma cidade cheia de gente em um carro roubado, e melhor deixar ele pra tras");
                } else {
                    mText->SetText("Ele precisou ejetar e sua nave caiu, agora ele precisa encontrar a os destrocos para pedir resgate e escapar do planeta desconhecido");
                }
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
                mState = CutsceneState::Finished;
                mGame->SetScene(mNextScene);
            }
            break;

        case CutsceneState::FadeOut3:
            mState = CutsceneState::Finished;
            mGame->SetScene(mNextScene);
            break;
            
        case CutsceneState::Finished:
            break;
    }
}

void CutsceneScreen::HandleKeyPress(int key)
{
    switch (mState)
    {
        case CutsceneState::Frame1:
            mState = CutsceneState::FadeOut1;
            mTimer = 0.0f;
            mFadeTimer = 0.0f;
            break;
            
        case CutsceneState::FadeOut1:
            mState = CutsceneState::FadeIn2;
            mFadeTimer = 0.0f;
            mImage1->SetIsVisible(false);
            mImage2->SetIsVisible(true);
            if (mNextScene == GameScene::Level2) {
                mText->SetText("Ele nao pensou 2 vezes e ja roubou");
            } else {
                mText->SetText("E nao percebeu a nave quebrando");
            }
            break;

        case CutsceneState::FadeIn2:
            mState = CutsceneState::Frame2;
            mTimer = 0.0f;
            mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
            break;

        case CutsceneState::Frame2:
            mState = CutsceneState::FadeOut2;
            mTimer = 0.0f;
            mFadeTimer = 0.0f;
            break;

        case CutsceneState::FadeOut2:
            mState = CutsceneState::FadeIn3;
            mFadeTimer = 0.0f;
            mImage2->SetIsVisible(false);
            mImage3->SetIsVisible(true);
            if (mNextScene == GameScene::Level2) {
                mText->SetText("Ele chegou em uma cidade cheia de gente em um carro roubado, e melhor deixar ele pra tras");
            } else {
                mText->SetText("Ele precisou ejetar e sua nave caiu, agora ele precisa encontrar a os destrocos para pedir resgate e escapar do planeta desconhecido");
            }
            break;

        case CutsceneState::FadeIn3:
            mState = CutsceneState::Frame3;
            mTimer = 0.0f;
            mFadeRect->SetColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
            break;

        case CutsceneState::Frame3:
            mState = CutsceneState::Finished;
            mGame->SetScene(mNextScene);
            break;

        case CutsceneState::FadeOut3:
            mState = CutsceneState::Finished;
            mGame->SetScene(mNextScene);
            break;
            
        default:
            break;
    }
}
