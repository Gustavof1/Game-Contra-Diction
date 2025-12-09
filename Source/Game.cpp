// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "Game.h"
#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "Components/Drawing/DrawComponent.h"
#include "Components/Physics/RigidBodyComponent.h"
#include "Components/Drawing/AnimatorComponent.h"
#include "Components/Physics/AABBColliderComponent.h"
#include "Components/ParticleSystemComponent.h"
#include "Random.h"
#include "Actors/Actor.h"
#include "Actors/Block.h"
#include "Actors/Goomba.h"
#include "Actors/Policeman.h"
#include "Actors/Spawner.h"
#include "Actors/Spaceman.h"
#include "Actors/AlienKid.h"
#include "Actors/AlienMan.h"
#include "Actors/AlienWoman.h"
#include "Actors/Mushroom.h"
#include "Actors/Coin.h"
#include "Actors/EndPhaseTrigger.h"
#include "Actors/Hazard.h"
#include "Actors/RobotFlyer.h"
#include "UI/Screens/MainMenu.h"
#include "UI/Screens/GameOver.h"
#include "UI/Screens/PauseMenu.h"
#include "UI/Screens/InventoryScreen.h"
#include "UI/Screens/LoadingScreen.h"
#include "UI/HUD.h"
#include "Actors/ParallaxActor.h"
#include "Actors/PlayerBullet.h"
#include "Actors/PolicemanBullet.h"
#include "Actors/RobotTurret.h"
#include "Actors/Grass.h"
#include "Actors/Stone.h"
#include "Components/Drawing/TutorialDrawComponent.h"
#include "Components/Drawing/SpriteComponent.h"
#include "Actors/Soldier.h"
#include "Actors/FinalFlower.h"
#include "Actors/FlowerBoss.h"
#include "Renderer/Font.h"

// Atalho para facilitar leitura do JSON
using json = nlohmann::json;

int Game::WINDOW_WIDTH = 1280;
int Game::WINDOW_HEIGHT = 768;

Game::Game()
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mIsDebugging(false)
        ,mUpdatingActors(false)
        ,mCameraPos(Vector2::Zero)
        ,mZoomScale(1.0f)
        ,mPlayer(nullptr)
        ,mLevelData(nullptr)
        ,mIsPlayerDead(false)
        ,mCoinCount(0)
        ,mState(GameState::Gameplay)
        ,mLevelWidth(0.0f)
        ,mLevelHeight(0.0f)
        ,mAmbientLight(0.3f, 0.3f, 0.3f)
        ,mIsImmortal(false)
        ,mIsLoading(false)
        ,mNextScene(GameScene::MainMenu)
        ,mFadeState(FadeState::None)
        ,mFadeTimer(0.0f)
{

}

bool Game::Initialize()
{
    Random::Init();

    mIsDebugging = false;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    // Ensure the working directory matches the executable directory so relative paths stay valid
    if (char* basePath = SDL_GetBasePath())
    {
        std::filesystem::current_path(basePath);
        SDL_free(basePath);
    }

    // Init SDL Image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        SDL_Log("Unable to initialize SDL_image: %s", IMG_GetError());
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() != 0)
    {
        SDL_Log("Failed to initialize SDL_ttf");
        return false;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        SDL_Log("Failed to initialize SDL_mixer");
        return false;
    }

    // Get screen dimensions
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        // Set window size to 90% of screen size to leave space for window bar
        WINDOW_WIDTH = static_cast<int>(dm.w * 0.9f);
        WINDOW_HEIGHT = static_cast<int>(dm.h * 0.9f);
    }

    mWindow = SDL_CreateWindow("Contra-Diction", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mLevelWidth = 0.0f;
    mLevelHeight = 0.0f;

    mRenderer = new Renderer(mWindow);
    mRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    mAudio = new AudioSystem();

    mHUD = new HUD(this);

    PlayMusic("Menu.ogg");

    SetScene(GameScene::MainMenu);

    mTicksCount = SDL_GetTicks();

    return true;
}

void Game::UnloadScene()
{
    // 1. Marcar todos os atores para destruição
    for (auto actor : mActors)
    {
        actor->SetState(ActorState::Destroy);
    }
    // Limpar lista de pendentes
    mPendingActors.clear();

    // 2. Limpar Drawables e Colliders
    mDrawables.clear();
    mColliders.clear();

    // Limpar UI Stack
    while (!mUIStack.empty()) {
        delete mUIStack.back();
        mUIStack.pop_back();
    }

    // 3. Limpar dados do Level (IMPORTANTE para evitar vazamento de memória)
    if (mLevelData)
    {
        // Precisamos saber a altura antiga para deletar corretamente.
        // Assumindo que mLevelHeight foi configurado corretamente no BuildLevel.
        int heightInTiles = static_cast<int>(mLevelHeight / Game::TILE_SIZE);

        // Proteção extra: se mLevelHeight for 0 (primeira execução), não deleta nada
        if (heightInTiles > 0) {
            for (int i = 0; i < heightInTiles; ++i)
            {
                delete[] mLevelData[i];
            }
            delete[] mLevelData;
        }
        mLevelData = nullptr;
    }

    // 4. Resetar ponteiros de gameplay
    mPlayer = nullptr;
    mIsPlayerDead = false;
    mCoinCount = 0;
}

void Game::SetScene(GameScene scene)
{
    // If we are already transitioning to the same scene, do not reset the timer
    if (mFadeState != FadeState::None && mNextScene == scene)
    {
        return;
    }

    mNextScene = scene;
    mFadeState = FadeState::FadeOut;
    mFadeTimer = 0.0f;
}

void Game::PerformLoad(GameScene scene)
{
    // Primeiro limpa a cena anterior
    UnloadScene();

    mPreviousScene = mCurrentScene;
    mCurrentScene = scene;

    switch (scene)
    {
        case GameScene::MainMenu:
        {
            PlayMusic("Contra(NES)SnowField(RemixSuno).mp3");
            new MainMenu(this, "../Assets/Fonts/ALS_Micro_Bold.ttf");
            break;
        }
        case GameScene::Level1:
        {
            PlayMusic("Contra(NES)AliensLairTheme(RemixSuno).mp3");

            // Use Screen Size for Parallax Layers
            float screenW = static_cast<float>(WINDOW_WIDTH);
            float screenH = static_cast<float>(WINDOW_HEIGHT);

            mAmbientLight = Vector3(0.3f, 0.3f, 0.3f);

            // Create parallax layers BEFORE level to ensure they are drawn behind if orders are equal
            
            // Sky: Factor 1.0 (Static) - Order 10
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/sky #edffe2.png", 1.0f, screenW, screenH, 10);
            
            // 5.png: Factor 0.9 - Order 20
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/5.png", 0.9f, screenW, screenH, 20);
            
            // 4.png: Factor 0.8 - Order 30
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/4.png", 0.8f, screenW, screenH, 30);
            
            // 3.png: Factor 0.7 - Order 40
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/3.png", 0.7f, screenW, screenH, 40);
            
            // 2.png: Factor 0.6 - Order 50
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/2.png", 0.6f, screenW, screenH, 50);
            
            // 1.png: Factor 0.5 - Order 60
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/1.png", 0.5f, screenW, screenH, 60);
            
            // Ground: Factor 0.0 (Moves with gameplay) - Order 100 (Same as Blocks/Player)
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/ground.png", 0.0f, screenW, screenH, 100);
            
            // Front: Factor -0.2 (Foreground) - Order 150
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/front.png", -0.2f, screenW, screenH, 150);

            // 1. Carrega o Nível (Player/Blocks created here, Order 100)
            // Since they are created AFTER Ground, and have same Order 100, they will be drawn AFTER Ground (on top).
            BuildLevelFromJSON("../Assets/Levels/Level1ContraDiction/Level1.tmj");

            // Add manual spawners for testing
            // auto* spawner1 = new Spawner(this, 600.0f, SpawnerType::AlienKid);
            // spawner1->SetPosition(Vector2(1000.0f, 400.0f));

            // auto* spawner2 = new Spawner(this, 600.0f, SpawnerType::AlienMan);
            // spawner2->SetPosition(Vector2(1500.0f, 400.0f));
            //
            // auto* spawner3 = new Spawner(this, 600.0f, SpawnerType::AlienWoman);
            // spawner3->SetPosition(Vector2(2000.0f, 400.0f));

            break;
        }
        case GameScene::Level2: {
            mAmbientLight = Vector3(1.0f, 1.0f, 1.0f);
            PlayMusic("CityTheme.ogg");

            // Use Screen Size for Parallax Layers
            float screenW = static_cast<float>(WINDOW_WIDTH);
            float screenH = static_cast<float>(WINDOW_HEIGHT);

            // Sky: Factor 1.0 (Static) - Order 10
            new ParallaxActor(this, "../Assets/Levels/Level2ContraDiction/City background sky.png", .05f, screenW, screenH, 10);

            // layer2.png: Factor 0.9 - Order 20 (Longe)
            new ParallaxActor(this, "../Assets/Levels/Level2ContraDiction/City background layer1.png", 0.5f, screenW, screenH, 20);

            // // layer1.png: Factor 0.2 - Order 30 (Perto)
            new ParallaxActor(this, "../Assets/Levels/Level2ContraDiction/City background layer2.png", 0.3f, screenW, screenH, 30);

            BuildLevelFromJSON("../Assets/Levels/Level2ContraDiction/Level2_City_Generated.tmj");

            // Spawn NPCs distributed throughout the map
            if (mLevelWidth > 0.0f) {
                std::vector<std::function<void(Vector2)>> spawners;
                
                // Helper to add spawners
                auto add = [&](std::function<void(Vector2)> creator) {
                    spawners.push_back(creator);
                };

                // Add 2 of each NPC type
                for(int i=0; i<2; i++) {
                    add([this](Vector2 p){ auto* a = new Policeman(this); a->SetPosition(p); });
                    add([this](Vector2 p){ auto* a = new Soldier(this); a->SetPosition(p); });
                    add([this](Vector2 p){ auto* a = new AlienKid(this); a->SetPosition(p); });
                    add([this](Vector2 p){ auto* a = new AlienMan(this); a->SetPosition(p); });
                    add([this](Vector2 p){ auto* a = new AlienWoman(this); a->SetPosition(p); });
                    add([this](Vector2 p){ auto* a = new RobotTurret(this); a->SetPosition(p); });
                    add([this](Vector2 p){ auto* a = new RobotFlyer(this); a->SetPosition(p); });
                }

                // Distribute them across the level
                // Start after the initial safe zone (e.g., 600px) and end before the very edge
                float startX = 800.0f; 
                float endX = mLevelWidth - 800.0f;
                
                if (endX > startX) {
                    float step = (endX - startX) / spawners.size();
                    
                    for (size_t i = 0; i < spawners.size(); ++i) {
                        float x = startX + i * step;
                        // Add some randomness to X
                        x += Random::GetFloatRange(-100.0f, 100.0f);
                        
                        // Y position: Spawn them high enough to fall to the ground
                        // Assuming the ground is roughly at the bottom of the level
                        float y = mLevelHeight - 300.0f; 
                        
                        spawners[i](Vector2(x, y));
                    }
                }
            }
            break;
        }
        case GameScene::Level3: {
            mAmbientLight = Vector3(0.8f, 0.8f, 0.8f);
            PlayMusic("Contra(NES)AliensLairTheme(RemixSuno).mp3"); // Or any other music

            // Use Screen Size for Parallax Layers
            float screenW = static_cast<float>(WINDOW_WIDTH);
            float screenH = static_cast<float>(WINDOW_HEIGHT);

            // Reuse Level 1 background for now or generic
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/sky #edffe2.png", 1.0f, screenW, screenH, 10);
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/5.png", 0.9f, screenW, screenH, 20);
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/4.png", 0.8f, screenW, screenH, 30);
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/3.png", 0.7f, screenW, screenH, 40);
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/2.png", 0.6f, screenW, screenH, 50);
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/1.png", 0.5f, screenW, screenH, 60);
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/ground.png", 0.0f, screenW, screenH, 100);
            new ParallaxActor(this, "../Assets/Sprites/Background-ContraDiction/jungle parallax background/front.png", -0.2f, screenW, screenH, 150);

            // Load Level 3
            BuildLevelFromJSON("../Assets/Levels/Level3ContraDiction/Level3.tmj");

            // Add FinalFlower closer to the start
            // mLevelWidth is set by BuildLevelFromJSON
            auto* flower = new FinalFlower(this);
            // Place on ground. Assuming ground is at bottom or similar to other levels.
            // If level is 15 tiles high (480px), place at bottom.
            // But BuildLevelFromJSON sets mLevelHeight.
            // Let's place it closer to start (e.g. 1000.0f) instead of middle.
            flower->SetPosition(Vector2(1000.0f, mLevelHeight - 200.0f)); // Adjust Y as needed

            break;
        }
        case GameScene::TestLevel:
        {
            mAmbientLight = Vector3(0.8f, 0.8f, 0.8f);
            PlayMusic("Level1.ogg");

            mLevelWidth = 3000.0f;
            mLevelHeight = static_cast<float>(WINDOW_HEIGHT);

            // Create a simple floor
            Texture* blockTex = mRenderer->GetTexture("../Assets/Sprites/Blocks/Assets.png");
            // Assuming 0,0 is a valid block tile in Assets.png
            // Let's check Assets.png... well I can't check image content.
            // But BuildLevelFromJSON uses it.
            
            // Create floor
            for (int i = 0; i < 100; i++) {
                Block* b = new Block(this, blockTex, 0, 0, 32, true);
                b->SetPosition(Vector2(i * 32.0f + 16.0f, WINDOW_HEIGHT - 16.0f));
            }

            // Player
            mPlayer = new Spaceman(this);
            mPlayer->SetPosition(Vector2(100.0f, WINDOW_HEIGHT - 100.0f));

            // NPCs
            auto* soldier = new Soldier(this);
            soldier->SetPosition(Vector2(600.0f, WINDOW_HEIGHT - 64.0f));

            auto* policeman = new Policeman(this);
            policeman->SetPosition(Vector2(900.0f, WINDOW_HEIGHT - 64.0f));
            
            auto* soldier2 = new Soldier(this);
            soldier2->SetPosition(Vector2(1300.0f, WINDOW_HEIGHT - 64.0f));

            auto* policeman2 = new Policeman(this);
            policeman2->SetPosition(Vector2(1600.0f, WINDOW_HEIGHT - 64.0f));

            break;
        }
        case GameScene::FinalLevel:
            PlayMusic("GrassFields.ogg");
            break;
        case GameScene::GameOver:
            PlayMusic("menu.ogg");
            new GameOver(this, "../Assets/Fonts/ALS_Micro_Bold.ttf", mPreviousScene);
            break;
    }
}

// Removi LoadLevel e BuildLevel antigos para evitar confusão

void Game::RunLoop()
{
    while (mIsRunning)
    {
        float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
        if (deltaTime > 0.05f)
        {
            deltaTime = 0.05f;
        }

        mTicksCount = SDL_GetTicks();

        ProcessInput();
        UpdateGame(deltaTime);
        GenerateOutput();

        // Cap frame rate
        int sleepTime = (1000 / FPS) - (SDL_GetTicks() - mTicksCount);
        if (sleepTime > 0)
        {
            SDL_Delay(sleepTime);
        }
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
            case SDL_MOUSEMOTION:
                if (!mUIStack.empty())
                {
                    mUIStack.back()->HandleMouseMove(Vector2(static_cast<float>(event.motion.x), static_cast<float>(event.motion.y)));
                }
                break;
            case SDL_MOUSEWHEEL:
                if (mState == GameState::Gameplay)
                {
                    if (event.wheel.y > 0) // Scroll up
                    {
                        mZoomScale += 0.1f;
                    }
                    else if (event.wheel.y < 0) // Scroll down
                    {
                        mZoomScale -= 0.1f;
                    }
                    mZoomScale = std::clamp(mZoomScale, MIN_ZOOM, MAX_ZOOM);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (!mUIStack.empty())
                {
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        mUIStack.back()->HandleMouseClick(Vector2(static_cast<float>(event.button.x), static_cast<float>(event.button.y)));
                    }
                }
                break;
            case SDL_KEYDOWN:
                if (!mUIStack.empty())
                {
                    mUIStack.back()->HandleKeyPress(event.key.keysym.sym);
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE && mState == GameState::Gameplay)
                {
                    new PauseMenu(this, "../Assets/Fonts/ALS_Micro_Bold.ttf", mCurrentScene);
                }
                else if (event.key.keysym.sym == SDLK_TAB && mState == GameState::Gameplay)
                {
                    new InventoryScreen(this);
                }
                else if (event.key.keysym.sym == SDLK_F11)
                {
                    Uint32 flags = SDL_GetWindowFlags(mWindow);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
                    {
                        SDL_SetWindowFullscreen(mWindow, 0);
                    }
                    else
                    {
                        SDL_SetWindowFullscreen(mWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                }
                else if ((event.key.keysym.sym == SDLK_EQUALS || event.key.keysym.sym == SDLK_KP_PLUS) && mState == GameState::Gameplay)
                {
                    mZoomScale += 0.1f;
                    mZoomScale = std::clamp(mZoomScale, MIN_ZOOM, MAX_ZOOM);
                }
                else if ((event.key.keysym.sym == SDLK_MINUS || event.key.keysym.sym == SDLK_KP_MINUS) && mState == GameState::Gameplay)
                {
                    mZoomScale -= 0.1f;
                    mZoomScale = std::clamp(mZoomScale, MIN_ZOOM, MAX_ZOOM);
                }
                break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);

    if (mState == GameState::Gameplay)  {
        for (auto actor : mActors) {
            actor->ProcessInput(state);
        }
    }
}

void Game::UpdateGame(float deltaTime)
{
    if (mFadeState == FadeState::FadeOut)
    {
        mFadeTimer += deltaTime;
        if (mFadeTimer >= FADE_TIME)
        {
            mFadeState = FadeState::FadeIn;
            mFadeTimer = 0.0f;

            if (mNextScene == GameScene::Level1 || mNextScene == GameScene::Level2 || mNextScene == GameScene::Level3 || mNextScene == GameScene::TestLevel) {
                mIsLoading = true;
                UnloadScene();
                new LoadingScreen(this);
                return;
            } else {
                PerformLoad(mNextScene);
            }
        }
    }
    else if (mFadeState == FadeState::FadeIn)
    {
        mFadeTimer += deltaTime;
        if (mFadeTimer >= FADE_TIME)
        {
            mFadeState = FadeState::None;
        }
    }

    if (mIsLoading) {
        PerformLoad(mNextScene);
        mIsLoading = false;
        // Close Loading Screen
        if (!mUIStack.empty()) {
             auto* screen = mUIStack.back();
             // Check if it is loading screen? Or just close top.
             // LoadingScreen is likely the only one or top one.
             screen->Close();
        }
        return;
    }

    if (mAudio) {
        mAudio->Update(deltaTime);
    }

    UpdateUI(deltaTime);

    // Update Floating Texts
    for (auto it = mFloatingTexts.begin(); it != mFloatingTexts.end(); ) {
        it->timer += deltaTime;
        if (it->timer >= it->duration) {
            delete it->texture;
            it = mFloatingTexts.erase(it);
        } else {
            ++it;
        }
    }

    float currentZoom = !mUIStack.empty() ? 1.0f : mZoomScale;
    for (auto pa : mParallaxActors) {
        pa->UpdateLayout(currentZoom);
    }

    if (!mUIStack.empty()) {
        for (auto ui : mUIStack) {
            ui->Update(deltaTime);
        }
    }
    else {
        UpdateActors(deltaTime);
    }
    UpdateCamera();
}

void Game::UpdateUI(float deltaTime)
{
    for (auto it = mUIStack.begin(); it != mUIStack.end(); ) {
        if ((*it)->GetState() == UIScreen::UIState::Closing) {
            delete *it;
            it = mUIStack.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Game::UpdateActors(float deltaTime)
{
    mUpdatingActors = true;
    for (auto actor : mActors)
    {
        actor->Update(deltaTime);
    }
    mUpdatingActors = false;

    for (auto pending : mPendingActors)
    {
        mActors.emplace_back(pending);
    }
    mPendingActors.clear();

    std::vector<Actor*> deadActors;
    for (auto actor : mActors)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            deadActors.emplace_back(actor);
        }
    }

    for (auto actor : deadActors)
    {
        delete actor;
    }
}

void Game::UpdateCamera()
{
    if (!mPlayer) return;

    float zoom = mUIStack.empty() ? mZoomScale : 1.0f;

    // Use as variáveis que preenchemos no JSON!
    const float levelPixelWidth = mLevelWidth;
    const float levelPixelHeight = mLevelHeight;

    const float viewportWidth = static_cast<float>(WINDOW_WIDTH) / zoom;
    const float viewportHeight = static_cast<float>(WINDOW_HEIGHT) / zoom;

    // --- LÓGICA VERTICAL ---
    // Se o mapa for MENOR que a tela (ex: 480px mapa vs 768px tela)
    if (levelPixelHeight < viewportHeight)
    {
        // Alinha o FUNDO do mapa com o FUNDO da tela.
        mCameraPos.y = levelPixelHeight - viewportHeight;
    }
    else
    {
        // Se o mapa for GRANDE (scroll vertical)
        // O usuário pediu para a câmera ser estática verticalmente para não quebrar o background no pulo.
        // Vamos travar a câmera no fundo do nível (assumindo que o chão é embaixo).
        mCameraPos.y = levelPixelHeight - viewportHeight;
    }

    // --- LÓGICA HORIZONTAL ---
    float maxScrollX = levelPixelWidth - viewportWidth;
    if (maxScrollX < 0) maxScrollX = 0;

    float targetX = mPlayer->GetPosition().x - (viewportWidth / 2.0f);
    mCameraPos.x = std::clamp(targetX, 0.0f, maxScrollX);
}

void Game::AddActor(Actor* actor)
{
    if (mUpdatingActors)
    {
        mPendingActors.emplace_back(actor);
    }
    else
    {
        mActors.emplace_back(actor);
    }
}

void Game::RemoveActor(Actor* actor)
{
    auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
    if (iter != mPendingActors.end())
    {
        std::iter_swap(iter, mPendingActors.end() - 1);
        mPendingActors.pop_back();
    }

    iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end())
    {
        std::iter_swap(iter, mActors.end() - 1);
        mActors.pop_back();
    }
}

void Game::AddParallaxActor(ParallaxActor* actor)
{
    mParallaxActors.emplace_back(actor);
}

void Game::RemoveParallaxActor(ParallaxActor* actor)
{
    auto iter = std::find(mParallaxActors.begin(), mParallaxActors.end(), actor);
    if (iter != mParallaxActors.end())
    {
        mParallaxActors.erase(iter);
    }
}

void Game::AddDrawable(class DrawComponent *drawable)
{
    mDrawables.emplace_back(drawable);

    std::stable_sort(mDrawables.begin(), mDrawables.end(),[](DrawComponent* a, DrawComponent* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Game::RemoveDrawable(class DrawComponent *drawable)
{
    auto iter = std::find(mDrawables.begin(), mDrawables.end(), drawable);
    if (iter != mDrawables.end()) {
        mDrawables.erase(iter);
    }
}

void Game::AddCollider(class AABBColliderComponent* collider)
{
    mColliders.emplace_back(collider);
}

void Game::RemoveCollider(AABBColliderComponent* collider)
{
    auto iter = std::find(mColliders.begin(), mColliders.end(), collider);
    if (iter != mColliders.end()) {
        mColliders.erase(iter);
    }
}

void Game::GenerateOutput()
{
    mRenderer->Clear();

    float zoom = mUIStack.empty() ? mZoomScale : 1.0f;
    mRenderer->SetZoom(zoom);

    // Setup Lighting
    std::vector<Vector2> lightPositions;
    std::vector<float> lightRadii;
    std::vector<Vector3> lightColors;

    if (mPlayer)
    {
        lightPositions.push_back(mPlayer->GetPosition());
        lightRadii.push_back(300.0f);
        lightColors.push_back(Vector3(1.0f, 1.0f, 1.0f));
    }

    for (auto actor : mActors)
    {
        if (dynamic_cast<PlayerBullet*>(actor) || dynamic_cast<PolicemanBullet*>(actor))
        {
            lightPositions.push_back(actor->GetPosition());
            lightRadii.push_back(100.0f);
            lightColors.push_back(Vector3(1.0f, 1.0f, 1.0f));
        }
        else if (dynamic_cast<Particle*>(actor))
        {
            // Only add light if particle is active (visible)
            if (actor->GetState() == ActorState::Active)
            {
                lightPositions.push_back(actor->GetPosition());
                lightRadii.push_back(100.0f);
                lightColors.push_back(Vector3(1.0f, 1.0f, 0.0f)); // Yellow tint
            }
        }
    }

    mRenderer->SetLightUniforms(lightPositions, lightRadii, lightColors, mAmbientLight);
    // SetLightUniforms sets the active shader to LightShader

    for (auto drawable : mDrawables)
    {
        drawable->Draw(mRenderer);

        if(mIsDebugging)
        {
              for (auto comp : drawable->GetOwner()->GetComponents())
              {
                comp->DebugDraw(mRenderer);
              }
        }
    }

    // Switch back to base shader for UI and Floating Text
    mRenderer->SetActiveShader(mRenderer->GetBaseShader());

    for (const auto& ft : mFloatingTexts) {
        Vector2 size(static_cast<float>(ft.texture->GetWidth()), static_cast<float>(ft.texture->GetHeight()));
        Vector2 drawPos = ft.pos;
        drawPos.y -= 80.0f; // Offset up
        
        mRenderer->DrawTexture(drawPos, size, 0.0f, Vector3::One, ft.texture, Vector4::UnitRect, mCameraPos);
    }
    
    mRenderer->SetZoom(1.0f);

    if (mHUD && mState == GameState::Gameplay) {
        mHUD->Draw(mRenderer);
    }

    mRenderer->DrawUI(mRenderer->GetBaseShader());

    if (mFadeState != FadeState::None)
    {
        float alpha = 0.0f;
        if (mFadeState == FadeState::FadeOut)
        {
            alpha = mFadeTimer / FADE_TIME;
        }
        else if (mFadeState == FadeState::FadeIn)
        {
            alpha = 1.0f - (mFadeTimer / FADE_TIME);
        }
        
        alpha = Math::Clamp(alpha, 0.0f, 1.0f);

        mRenderer->DrawRect(Vector2(WINDOW_WIDTH/2.0f, WINDOW_HEIGHT/2.0f), Vector2(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)), 0.0f, Vector3(0.0f, 0.0f, 0.0f), Vector2::Zero, RendererMode::TRIANGLES, alpha);
    }

    mRenderer->Present();
}

void Game::AddFloatingText(const Vector2& pos, const std::string& text, float duration)
{
    Font* font = mRenderer->GetFont("../Assets/Fonts/ALS_Micro_Bold.ttf");
    if (font) {
        Texture* tex = font->RenderText(text, Vector3(1.0f, 1.0f, 1.0f), 20);
        if (tex) {
            FloatingText ft;
            ft.pos = pos;
            ft.text = text;
            ft.duration = duration;
            ft.timer = 0.0f;
            ft.texture = tex;
            mFloatingTexts.push_back(ft);
        }
    }
}

void Game::OnNPCKilled(Actor* actor)
{
    // Only spawn drones if the player is alive
    if (!mPlayer || mIsPlayerDead) return;

    // Spawn a flock of 4-5 drones
    int count = 4 + (rand() % 2); // 4 or 5
    
    Vector2 playerPos = mPlayer->GetPosition();
    
    // Spawn them off-screen (top-right or top-left depending on player direction or random)
    // Let's spawn them high up and slightly ahead or behind
    float spawnX = playerPos.x + (rand() % 2 == 0 ? 600.0f : -600.0f);
    float spawnY = playerPos.y - 200.0f; // Lower spawn height (was 500.0f)

    for (int i = 0; i < count; ++i) {
        auto* drone = new RobotFlyer(this);
        // Add some randomness to initial position
        Vector2 offset(static_cast<float>(rand() % 100 - 50), static_cast<float>(rand() % 100 - 50));
        drone->SetPosition(Vector2(spawnX, spawnY) + offset);
        
        // Force activation if needed, or rely on distance check
        // If spawnX is 600 away, it is within 700 activation range.
    }
    
    // Optional: Add text
    // AddFloatingText(actor->GetPosition(), "ALERT!", 1.0f);
}

void Game::Shutdown()
{
    UnloadScene(); // Limpa a cena atual

    while (!mActors.empty()) {
        delete mActors.back();
    }

    if (mAudio) {
        delete mAudio;
        mAudio = nullptr;
    }

    if (mHUD) {
        delete mHUD;
        mHUD = nullptr;
    }

    mRenderer->Shutdown();
    delete mRenderer;
    mRenderer = nullptr;

    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void Game::PlayMusic(const std::string& musicName)
{
    if (mMusicHandle.IsValid())
    {
        mAudio->StopSound(mMusicHandle);
    }

    mMusicHandle = mAudio->PlaySound(musicName, true, SoundCategory::Music);
}

void Game::BuildLevelFromJSON(const std::string& fileName)
{
    const unsigned int FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
    const unsigned int FLIPPED_VERTICALLY_FLAG   = 0x40000000;
    const unsigned int FLIPPED_DIAGONALLY_FLAG   = 0x20000000;

    mLevelData = nullptr;

    std::ifstream file(fileName);
    if (!file.is_open()) {
        SDL_Log("Failed to load JSON level: %s", fileName.c_str());
        return;
    }

    json mapData;
    file >> mapData;

    int width = mapData["width"];
    int height = mapData["height"];
    int tileWidth = mapData["tilewidth"];

    std::filesystem::path levelPath(fileName);
    std::filesystem::path levelDir = levelPath.parent_path();

    mLevelWidth = static_cast<float>(width * tileWidth);
    mLevelHeight = static_cast<float>(height * tileWidth);

    std::string tilesetName = "../Assets/Sprites/Blocks/Assets.png";
    Texture* tilesetTexture = mRenderer->GetTexture(tilesetName);

    mLevelData = new int*[height];
    for (int i = 0; i < height; ++i) {
        mLevelData[i] = new int[width];
        for (int k = 0; k < width; k++) mLevelData[i][k] = -1;
    }

    auto& layers = mapData["layers"];
    auto& tilesets = mapData["tilesets"];

    // -------------------------------------------------------
    // PASSO 1: PROCESSAR O CHÃO (TILE LAYERS)
    // -------------------------------------------------------
    for (const auto& layer : layers) {
        if (layer["type"] == "tilelayer" && layer["name"] != "Background") {
            std::string layerName = layer["name"];

            bool isCollidable = !(
                layerName == "Cenario" ||
                layerName == "Decoracao" ||
                layerName == "Lixos" ||
                layerName == "Postes" ||
                layerName == "Placas"  ||
                layerName == "Fundo"
            );

            std::vector<long long> data = layer["data"].get<std::vector<long long>>();

            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    long long rawVal = data[i * width + j];
                    unsigned int rawID = static_cast<unsigned int>(rawVal);

                    // 1. Ler as flags
                    bool flipH = (rawID & FLIPPED_HORIZONTALLY_FLAG);
                    bool flipV = (rawID & FLIPPED_VERTICALLY_FLAG);
                    bool flipD = (rawID & FLIPPED_DIAGONALLY_FLAG);

                    // 2. Limpar o ID para pegar a imagem
                    unsigned int tileID = rawID & ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG);

                    if (tileID != 0) {
                        if (isCollidable) {
                            mLevelData[i][j] = tileID;
                        }

                        json activeTileset;
                        bool found = false;
                        int maxFgidFound = -1;

                        for (const auto& ts : tilesets) {
                            int fgid = ts["firstgid"];
                            if (tileID >= fgid && fgid > maxFgidFound) {
                                activeTileset = ts;
                                maxFgidFound = fgid;
                                found = true;
                            }
                        }

                        if (!found) continue;

                        std::string imgJsonPath = activeTileset["image"];
                        std::filesystem::path fullTexturePath = levelDir / imgJsonPath;
                        std::string finalPathString = fullTexturePath.string();

                        Texture* tex = mRenderer->GetTexture(finalPathString);

                        if (!tex) {
                            SDL_Log("Textura nao encontrada em: %s", finalPathString.c_str());
                            continue;
                        }

                        int firstGid = activeTileset["firstgid"];
                        int mapTileWidth = mapData["tilewidth"]; // 16
                        int mapTileHeight = mapData["tileheight"]; // 16
                        int imageWidth = activeTileset["imagewidth"]; // Largura da imagem deste tileset
                        int columns = imageWidth / mapTileWidth;

                        int actualID = tileID - firstGid;
                        int srcX = (actualID % columns) * mapTileWidth;
                        int srcY = (actualID / columns) * mapTileHeight;

                        Vector2 pos(j * mapTileWidth + mapTileWidth / 2.0f, i * mapTileHeight + mapTileHeight / 2.0f);

                        Block* block = new Block(this, tex, srcX, srcY, mapTileWidth, isCollidable);
                        block->SetPosition(pos);
                        block->SetTexturePath(finalPathString);

                        // 3. CALCULAR ROTAÇÃO E FLIP BASEADO NO TILED
                        float rotation = 0.0f;
                        Vector2 scale(1.0f, 1.0f);

                        // A lógica do Tiled para rotação é baseada em trocas de eixos (Diagonal)
                        if (flipD) {
                            if (flipH && flipV) {
                                rotation = Math::Pi / 2.0f; // 90 graus
                                scale.x = 1.0f;
                                scale.y = -1.0f; // Ajuste necessário para alinhar com coords OpenGL
                            }
                            else if (flipH) {
                                rotation = Math::Pi / 2.0f; // 90 graus
                                scale.x = 1.0f;
                                scale.y = 1.0f;
                            }
                            else if (flipV) {
                                rotation = 3.0f * Math::Pi / 2.0f; // 270 graus
                                scale.x = 1.0f;
                                scale.y = 1.0f;
                            }
                            else {
                                rotation = Math::Pi / 2.0f; // 90 graus
                                scale.x = -1.0f;
                                scale.y = 1.0f;
                            }
                        }
                        else {
                            // Sem diagonal, é apenas flip simples
                            if (flipH) scale.x = -1.0f;
                            if (flipV) scale.y = -1.0f;
                        }

                        // Aplica no bloco
                        block->SetFlipData(rotation, scale);
                    }
                }
            }
        }
    }
    

    for (const auto& layer : layers)
    {
        if (layer["type"] == "objectgroup")
        {
            for (const auto& object : layer["objects"])
            {
                std::string name = object["name"];
                float x = object["x"];
                float y = object["y"];
                float w = object.contains("width") ? object["width"].get<float>() : 0.0f;
                float h = object.contains("height") ? object["height"].get<float>() : 0.0f;

                if (object.contains("gid")) {
                    y -= h;
                }

                Vector2 finalPos(x + w / 2.0f, y + h / 2.0f);

                if (name == "PlayerStart") {
                    mPlayer = new Spaceman(this);
                    mPlayer->SetPosition(finalPos);
                    if (mCurrentScene == GameScene::Level1) {
                        new TutorialDrawComponent(mPlayer);
                    }
                } else if (mCurrentScene == GameScene::Level3 && (name == "Policeman" || name == "Robot" || name == "RobotFlyer" || name == "AlienKid" || name == "AlienMan" || name == "AlienWoman" || name == "Soldier" )) {
                    // Skip enemies in Level 3
                    continue;
                }
                // else if (name == "Policeman") {
                //      auto* enemy = new Policeman(this);
                //      enemy->SetPosition(finalPos);
                // }
                else if (name == "Hazard") {
                    auto* hazard = new Hazard(this, w, h);
                    hazard->SetPosition(finalPos);
                }
                else if (name == "Robot") {
                    auto* enemy = new Spawner(this, 600.0f, SpawnerType::RobotTurret);
                    enemy->SetPosition(finalPos);
                }
                // else if (name == "RobotFlyer") {
                //     // Cria 5 SPAWNERS. Cada um vai gerar um robô quando o player chegar perto.
                //     for (int i = 0; i < 5; i++) {
                //         // Cria o Spawner (invisível)
                //         auto* enemySpawner = new Spawner(this, 600.0f, SpawnerType::RobotFlyer);
                //
                //         // Calcula onde esse robô deve "estacionar" (Target)
                //         float targetOffsetX = (rand() % 100) - 50.0f;
                //         float targetOffsetY = (rand() % 100) - 50.0f;
                //         Vector2 targetPosition = finalPos + Vector2(targetOffsetX, targetOffsetY);
                //
                //         // Posiciona o SPAWNER no destino final.
                //         // Quando o robô nascer, ele vai querer vir para cá.
                //         enemySpawner->SetPosition(targetPosition);
                //     }
                // }
                else if (name == "AlienKid") {
                     auto* enemy = new Spawner(this, 600.0f, SpawnerType::AlienKid);
                     enemy->SetPosition(finalPos);
                } else if (name == "AlienMan") {
                    auto* enemy = new Spawner(this, 600.0f, SpawnerType::AlienMan);
                    enemy->SetPosition(finalPos);
                } else if (name == "AlienWoman") {
                    auto* enemy = new Spawner(this, 600.0f, SpawnerType::AlienWoman);
                    enemy->SetPosition(finalPos);
                }
                // else if (name == "Soldier") {
                //      auto* enemy = new Soldier(this);
                //      enemy->SetPosition(finalPos);
                // }
                else if (name == "EndPhase") {
                    // Cria o gatilho invisível no lugar do retângulo
                    auto* trigger = new EndPhaseTrigger(this, w, h);
                    trigger->SetPosition(finalPos);
                }
            }
        }
    }
}
void Game::SetGameOverInfo(Actor* killer)
{
    mGameOverInfo = GameOverInfo(); // Reset
    if (!killer) return;

    // Check if it's an enemy (has AnimatorComponent usually)
    auto* anim = killer->GetComponent<AnimatorComponent>();
    if (anim)
    {
        mGameOverInfo.isEnemy = true;
        mGameOverInfo.killerName = "Enemy"; 
        mGameOverInfo.killerSpritePath = anim->GetTexturePath();
        mGameOverInfo.killerJsonPath = anim->GetDataPath();
        return;
    }

    // Check if it's an enemy with SpriteComponent (e.g. Turret)
    auto* sprite = killer->GetComponent<SpriteComponent>();
    if (sprite && sprite->GetTexture())
    {
        mGameOverInfo.isBlock = true; // Treat as simple sprite (like block)
        mGameOverInfo.killerName = "Enemy";
        mGameOverInfo.killerSpritePath = sprite->GetTexture()->GetFileName();
        mGameOverInfo.useSrcRect = false; 
        return;
    }
    
    // Check if it's a block
    auto* block = dynamic_cast<Block*>(killer);
    if (block)
    {
        mGameOverInfo.isBlock = true;
        mGameOverInfo.killerName = "Block";
        mGameOverInfo.killerSpritePath = block->GetTexturePath();
        mGameOverInfo.useSrcRect = true;
        mGameOverInfo.srcX = block->GetOriginalSrcX();
        mGameOverInfo.srcY = block->GetOriginalSrcY();
        mGameOverInfo.srcW = block->GetSize();
        mGameOverInfo.srcH = block->GetSize();
        return;
    }

    // Check if it's a Hazard (Trap) - Try to find a block underneath
    if (dynamic_cast<Hazard*>(killer))
    {
        Vector2 killerPos = killer->GetPosition();
        for (auto* drawable : mDrawables) {
            auto* b = dynamic_cast<Block*>(drawable->GetOwner());
            if (b) {
                 // Check distance (assuming 32x32 blocks)
                 Vector2 diff = b->GetPosition() - killerPos;
                 if (diff.LengthSq() < 1600.0f) { // 40x40 distance squared
                     mGameOverInfo.isBlock = true;
                     mGameOverInfo.killerName = "Trap";
                     mGameOverInfo.killerSpritePath = b->GetTexturePath();
                     mGameOverInfo.useSrcRect = true;
                     mGameOverInfo.srcX = b->GetOriginalSrcX();
                     mGameOverInfo.srcY = b->GetOriginalSrcY();
                     mGameOverInfo.srcW = b->GetSize();
                     mGameOverInfo.srcH = b->GetSize();
                     return;
                 }
            }
        }
    }
}

void Game::SetLevelComplete(Actor* endActor)
{
    // Pause all active actors except the end sequence actor
    for (auto actor : mActors)
    {
        if (actor != endActor && actor->GetState() == ActorState::Active)
        {
            actor->SetState(ActorState::Paused);
        }
    }

    // Also handle pending actors
    for (auto actor : mPendingActors)
    {
        if (actor != endActor && actor->GetState() == ActorState::Active)
        {
            actor->SetState(ActorState::Paused);
        }
    }
}
