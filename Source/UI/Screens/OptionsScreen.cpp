#include "OptionsScreen.h"
#include "../../Game.h"
#include "../../AudioSystem.h"
#include "MainMenu.h"
#include <SDL.h>

OptionsScreen::OptionsScreen(Game* game, const std::string& fontName)
    : UIScreen(game, fontName)
    , mSFXSampleCooldown(0.0f)
{
    Vector2 screenCenter(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT / 2.0f);

    // Background
    auto bg = AddImage("../Assets/Menus/menu.png", screenCenter, 1.0f);
    bg->SetSize(Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)));
    bg->SetColor(Vector4(0.2f, 0.2f, 0.2f, 1.0f));

    // Title
    auto title = AddText("OPTIONS", Vector2(Game::WINDOW_WIDTH / 2.0f, Game::WINDOW_HEIGHT * 0.15f), 1.5f);
    title->SetTextColor(Vector3(1.0f, 0.6f, 0.0f));

    // Get current volumes
    int sfxVol = game->GetAudio()->GetBusVolume(SoundCategory::SFX);
    int musicVol = game->GetAudio()->GetBusVolume(SoundCategory::Music);
    mSFXVolumeVal = static_cast<float>(sfxVol) / 128.0f;
    mMusicVolumeVal = static_cast<float>(musicVol) / 128.0f;

    // SFX Slider
    AddText("SFX Volume", Vector2(Game::WINDOW_WIDTH * 0.3f, Game::WINDOW_HEIGHT * 0.4f), 1.0f);
    mSFXBar = AddRect(Vector2(Game::WINDOW_WIDTH * 0.6f, Game::WINDOW_HEIGHT * 0.4f), Vector2(300.0f, 20.0f));
    mSFXBar->SetColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
    
    mSFXKnob = AddRect(Vector2::Zero, Vector2(20.0f, 40.0f)); // Position set in UpdateKnobPositions
    mSFXKnob->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

    // Music Slider
    AddText("Music Volume", Vector2(Game::WINDOW_WIDTH * 0.3f, Game::WINDOW_HEIGHT * 0.6f), 1.0f);
    mMusicBar = AddRect(Vector2(Game::WINDOW_WIDTH * 0.6f, Game::WINDOW_HEIGHT * 0.6f), Vector2(300.0f, 20.0f));
    mMusicBar->SetColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f));

    mMusicKnob = AddRect(Vector2::Zero, Vector2(20.0f, 40.0f));
    mMusicKnob->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

    UpdateKnobPositions();

    // Back Button
    auto backBtn = AddButton("Back", [this]() {
        Close();
        new MainMenu(mGame, "../Assets/Fonts/ALS_Micro_Bold.ttf");
    }, Vector2(Game::WINDOW_WIDTH * 0.5f, Game::WINDOW_HEIGHT * 0.85f), 0.5f);
    
    // Use exit button textures for now as placeholder, or just text if textures not found
    backBtn->SetTextures("../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_normal.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_hover.png",
                         "../Assets/Sprites/MenuButtons-ContraDiction/Buttons/Medium/exit_pressed.png");
}

OptionsScreen::~OptionsScreen()
{
}

void OptionsScreen::Update(float deltaTime)
{
    UIScreen::Update(deltaTime);
    if (mSFXSampleCooldown > 0.0f)
    {
        mSFXSampleCooldown -= deltaTime;
    }
}

void OptionsScreen::UpdateKnobPositions()
{
    // SFX
    float sfxX = (mSFXBar->GetOffset().x - 150.0f) + (mSFXVolumeVal * 300.0f);
    mSFXKnob->SetOffset(Vector2(sfxX, mSFXBar->GetOffset().y));

    // Music
    float musicX = (mMusicBar->GetOffset().x - 150.0f) + (mMusicVolumeVal * 300.0f);
    mMusicKnob->SetOffset(Vector2(musicX, mMusicBar->GetOffset().y));
}

void OptionsScreen::HandleMouseClick(const Vector2& mousePos)
{
    UIScreen::HandleMouseClick(mousePos);

    // Check SFX Slider
    Vector2 sfxBarPos = mSFXBar->GetOffset();
    if (mousePos.x >= sfxBarPos.x - 160.0f && mousePos.x <= sfxBarPos.x + 160.0f &&
        mousePos.y >= sfxBarPos.y - 30.0f && mousePos.y <= sfxBarPos.y + 30.0f)
    {
        float relativeX = mousePos.x - (sfxBarPos.x - 150.0f);
        mSFXVolumeVal = Math::Clamp(relativeX / 300.0f, 0.0f, 1.0f);
        UpdateKnobPositions();
        mGame->GetAudio()->SetBusVolume(SoundCategory::SFX, static_cast<int>(mSFXVolumeVal * 128.0f));
        
        if (mSFXSampleCooldown <= 0.0f) {
            mGame->GetAudio()->PlaySound("Bump.wav"); // Play sample
            mSFXSampleCooldown = 0.2f;
        }
    }

    // Check Music Slider
    Vector2 musicBarPos = mMusicBar->GetOffset();
    if (mousePos.x >= musicBarPos.x - 160.0f && mousePos.x <= musicBarPos.x + 160.0f &&
        mousePos.y >= musicBarPos.y - 30.0f && mousePos.y <= musicBarPos.y + 30.0f)
    {
        float relativeX = mousePos.x - (musicBarPos.x - 150.0f);
        mMusicVolumeVal = Math::Clamp(relativeX / 300.0f, 0.0f, 1.0f);
        UpdateKnobPositions();
        mGame->GetAudio()->SetBusVolume(SoundCategory::Music, static_cast<int>(mMusicVolumeVal * 128.0f));
    }
}

void OptionsScreen::HandleMouseMove(const Vector2& mousePos)
{
    UIScreen::HandleMouseMove(mousePos);
    
    int x, y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) == 0) return;

    // Reuse click logic for drag
    HandleMouseClick(mousePos);
}
