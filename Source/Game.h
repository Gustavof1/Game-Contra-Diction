// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#pragma once
#include <SDL.h>
#include <vector>
#include "AudioSystem.h"
#include "Renderer/Renderer.h"
#include "UI/Screens/UIScreen.h"
#include <SDL_mixer.h>
#include "./Json.h"

enum class GameState
{
    Gameplay,
    Paused,
    Quit
};

enum class GameScene
{
    MainMenu,
    Level1,
    Level2,
    Level3,
    TestLevel,
    FinalLevel,
    GameOver
};

struct FloatingText {
    Vector2 pos;
    std::string text;
    float duration;
    float timer;
    class Texture* texture;
};

class Spaceman;

class Game
{
public:
    Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();
    void Quit() { mIsRunning = false; }

    // Actor functions
    //void InitializeActors();
    void UpdateActors(float deltaTime);
    void AddActor(class Actor* actor);
    void RemoveActor(class Actor* actor);

    // UI functions
    void PushUI(class UIScreen* screen) { mUIStack.emplace_back(screen); }
    const std::vector<class UIScreen*>& GetUIStack() { return mUIStack; }

    // Audio
    AudioSystem* GetAudio() { return mAudio; }
    SoundHandle GetMusicHandle() const { return mMusicHandle; }

    // Scene Handling
    void SetScene(GameScene scene);
    void PerformLoad(GameScene scene);
    void UnloadScene();

    // Game State
    GameState GetState() const { return mState; }
    void SetState(GameState state) { mState = state; }

    // Renderer
    class Renderer* GetRenderer() { return mRenderer; }

    static int WINDOW_WIDTH;
    static int WINDOW_HEIGHT;
    static const int LEVEL_WIDTH    = 215;
    static const int LEVEL_HEIGHT   = 15;
    static const int TILE_SIZE      = 32;
    static const int SPAWN_DISTANCE = 700;
    static const int FPS = 60;

    // Draw functions
    void AddDrawable(class DrawComponent* drawable);
    void RemoveDrawable(class DrawComponent* drawable);
    std::vector<class DrawComponent*>& GetDrawables() { return mDrawables; }

    // Collider functions
    void AddCollider(class AABBColliderComponent* collider);
    void RemoveCollider(class AABBColliderComponent* collider);
    std::vector<class AABBColliderComponent*>& GetColliders() { return mColliders; }

    // Camera functions
    Vector2& GetCameraPos() { return mCameraPos; };
    void SetCameraPos(const Vector2& position) { mCameraPos = position; };

    // Game specific
    const class Spaceman* GetPlayer() const { return mPlayer; }
    class Spaceman* GetPlayer() { return mPlayer; }
    void SetPlayerIsDead(bool isDead) { mIsPlayerDead = isDead; }
    void AddCoin() { mCoinCount++; }

    float GetLevelHeight() const { return mLevelHeight; }

    void SetImmortal(bool immortal) { mIsImmortal = immortal; }
    bool IsImmortal() const { return mIsImmortal; }

    void BuildLevelFromJSON(const std::string& fileName);

    void AddFloatingText(const Vector2& pos, const std::string& text, float duration);

    void OnNPCKilled(class Actor* actor);

    GameScene GetCurrentScene() const { return mCurrentScene; }

private:
    void ProcessInput();
    void UpdateGame(float deltaTime);
    void UpdateCamera();
    void GenerateOutput();

    void UpdateUI(float deltaTime);

    void PlayMusic(const std::string& musicName);

    // Level loading
    int **LoadLevel(const std::string& fileName, int width, int height);
    void BuildLevel(int** levelData, int width, int height);

    // All the actors in the game
    std::vector<class Actor*> mActors;
    std::vector<class Actor*> mPendingActors;

    // Camera
    Vector2 mCameraPos;

    // Audio system
    AudioSystem* mAudio;
    SoundHandle mMusicHandle;

    // HUD
    class HUD* mHUD;

    // All UI screens in the game
    std::vector<class UIScreen*> mUIStack;

    // All the draw components
    std::vector<class DrawComponent*> mDrawables;

    // All the collision components
    std::vector<class AABBColliderComponent*> mColliders;

    // SDL stuff
    SDL_Window* mWindow;
    class Renderer* mRenderer;

    // Track elapsed time since game start
    Uint32 mTicksCount;

    // Track if we're updating actors right now
    bool mIsRunning;
    bool mIsDebugging;
    bool mUpdatingActors;

    // Game-specific
    GameScene mCurrentScene;
    GameScene mPreviousScene;
    GameState mState;
    class Spaceman *mPlayer;
    int **mLevelData;
    bool mIsPlayerDead;
    int mCoinCount;

    float mLevelWidth;
    float mLevelHeight;

    Vector3 mAmbientLight;
    bool mIsImmortal;

    std::vector<FloatingText> mFloatingTexts;

    // Loading
    bool mIsLoading;
    GameScene mNextScene;
};
