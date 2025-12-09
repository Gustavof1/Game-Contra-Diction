#include "TutorialDrawComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"

TutorialDrawComponent::TutorialDrawComponent(Actor* owner)
    : DrawComponent(owner, 200) // Higher draw order to be on top
    , mShowJump(true)
    , mShowMove(true)
    , mShowCrouch(true)
    , mShowShoot(true)
    , mShowDance(true)
    , mShowInventory(true)
    , mShowRun(true)
{
    Renderer* renderer = owner->GetGame()->GetRenderer();

    // Load Key Textures
    mTexW = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/w.png");
    mTexA = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/a.png");
    mTexD = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/d.png");
    mTexS = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/s.png");
    mTexSpace = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/space.png");
    mTexUp = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/up.png");
    mTexDown = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/down.png");
    mTexLeft = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/left.png");
    mTexRight = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/right.png");
    mTexCtrl = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/ctrl.png");
    mTexMouseLeft = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/mouse/normal_horizontal/left.png");
    mTexTab = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/tab.png");
    mTexShift = renderer->GetTexture("../Assets/Sprites/InputButtons-ContraDiction/output/dark/keyboard/normal_horizontal/shift.png");

    // Load Font and Create Text Textures
    mFont = new Font();
    mFont->Load("../Assets/Fonts/ALS_Micro_Bold.ttf");

    mTexJump = mFont->RenderText("Jump", Vector3(1.0f, 1.0f, 1.0f), 20);
    mTexMove = mFont->RenderText("Move", Vector3(1.0f, 1.0f, 1.0f), 20);
    mTexCrouch = mFont->RenderText("Crouch", Vector3(1.0f, 1.0f, 1.0f), 20);
    mTexShoot = mFont->RenderText("Shoot", Vector3(1.0f, 0.0f, 0.0f), 20); // Red for Shoot
    mTexDance = mFont->RenderText("Dance", Vector3(1.0f, 1.0f, 1.0f), 20);
    mTexInventory = mFont->RenderText("Inventory", Vector3(1.0f, 1.0f, 1.0f), 20);
    mTexRun = mFont->RenderText("Run", Vector3(1.0f, 1.0f, 1.0f), 20);
}

TutorialDrawComponent::~TutorialDrawComponent()
{
    if (mFont)
    {
        mFont->Unload();
        delete mFont;
    }

    if (mTexJump) delete mTexJump;
    if (mTexMove) delete mTexMove;
    if (mTexCrouch) delete mTexCrouch;
    if (mTexShoot) delete mTexShoot;
    if (mTexDance) delete mTexDance;
    if (mTexInventory) delete mTexInventory;
    if (mTexRun) delete mTexRun;
}

void TutorialDrawComponent::ProcessInput(const uint8_t* keyState)
{
    if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_SPACE] || keyState[SDL_SCANCODE_UP])
    {
        mShowJump = false;
    }
    if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_LEFT] || keyState[SDL_SCANCODE_RIGHT])
    {
        mShowMove = false;
    }
    if (keyState[SDL_SCANCODE_LCTRL] || keyState[SDL_SCANCODE_RCTRL])
    {
        mShowCrouch = false;
    }
    if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
    {
        mShowDance = false;
    }
    if (keyState[SDL_SCANCODE_TAB])
    {
        mShowInventory = false;
    }
    if (keyState[SDL_SCANCODE_LSHIFT] || keyState[SDL_SCANCODE_RSHIFT])
    {
        mShowRun = false;
    }

    int x, y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    if (buttons & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        mShowShoot = false;
    }
}

void TutorialDrawComponent::Draw(Renderer* renderer)
{
    if (!mIsVisible) return;

    // Switch to BaseShader to ignore lighting
    // Shader* oldShader = renderer->GetActiveShader(); 
    
    renderer->SetActiveShader(renderer->GetBaseShader());

    // If all are hidden, we can stop drawing or even destroy the component (but let's just hide for now)
    if (!mShowJump && !mShowMove && !mShowCrouch && !mShowShoot && !mShowDance && !mShowInventory && !mShowRun)
    {
        renderer->SetActiveShader(renderer->GetLightShader()); // Restore
        return;
    }

    Vector2 pos = mOwner->GetPosition();
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();

    // Base offset above player - Higher as requested
    float startY = pos.y - 250.0f; // Moved up even more to accommodate top item
    float startX = pos.x;

    // Size for keys
    Vector2 keySize(24.0f, 24.0f);
    Vector2 spaceSize(48.0f, 24.0f); // Space is wider
    Vector2 ctrlSize(36.0f, 24.0f);
    Vector2 mouseSize(24.0f, 24.0f);
    float textScale = 0.75f;
    float rowSpacing = 40.0f; // More spaced

    float currentY = startY;

    // Row 0: [Tab] "Inventory" (Moved to top)
    if (mShowInventory)
    {
        float currentX = startX - 30.0f;
        if (mTexTab)
        {
            renderer->DrawTexture(Vector2(currentX, currentY), ctrlSize, 0.0f, Vector3::One, mTexTab, Vector4::UnitRect, cameraPos);
        }
        currentX += 70.0f;

        if (mTexInventory)
        {
            Vector2 textSize(mTexInventory->GetWidth() * textScale, mTexInventory->GetHeight() * textScale);
            renderer->DrawTexture(Vector2(currentX, currentY), textSize, 0.0f, Vector3::One, mTexInventory, Vector4::UnitRect, cameraPos);
        }
        currentY += rowSpacing;
    }

    // Row 1: [W] [Space] [Up] "Jump"
    if (mShowJump)
    {
        float currentX = startX - 60.0f; // Start more to the left to center
        
        if (mTexW) renderer->DrawTexture(Vector2(currentX, currentY), keySize, 0.0f, Vector3::One, mTexW, Vector4::UnitRect, cameraPos);
        currentX += 40.0f;
        
        if (mTexSpace) renderer->DrawTexture(Vector2(currentX, currentY), spaceSize, 0.0f, Vector3::One, mTexSpace, Vector4::UnitRect, cameraPos);
        currentX += 50.0f;
        
        if (mTexUp) renderer->DrawTexture(Vector2(currentX, currentY), keySize, 0.0f, Vector3::One, mTexUp, Vector4::UnitRect, cameraPos);
        currentX += 60.0f;

        if (mTexJump)
        {
            Vector2 textSize(mTexJump->GetWidth() * textScale, mTexJump->GetHeight() * textScale);
            renderer->DrawTexture(Vector2(currentX, currentY), textSize, 0.0f, Vector3::One, mTexJump, Vector4::UnitRect, cameraPos);
        }
        currentY += rowSpacing;
    }

    // Row 2: [A] [D] [Left] [Right] "Move"
    if (mShowMove)
    {
        float currentX = startX - 80.0f;

        if (mTexA) renderer->DrawTexture(Vector2(currentX, currentY), keySize, 0.0f, Vector3::One, mTexA, Vector4::UnitRect, cameraPos);
        currentX += 30.0f;

        if (mTexD) renderer->DrawTexture(Vector2(currentX, currentY), keySize, 0.0f, Vector3::One, mTexD, Vector4::UnitRect, cameraPos);
        currentX += 30.0f;

        if (mTexLeft) renderer->DrawTexture(Vector2(currentX, currentY), keySize, 0.0f, Vector3::One, mTexLeft, Vector4::UnitRect, cameraPos);
        currentX += 30.0f;

        if (mTexRight) renderer->DrawTexture(Vector2(currentX, currentY), keySize, 0.0f, Vector3::One, mTexRight, Vector4::UnitRect, cameraPos);
        currentX += 60.0f;
        
        if (mTexMove)
        {
            Vector2 textSize(mTexMove->GetWidth() * textScale, mTexMove->GetHeight() * textScale);
            renderer->DrawTexture(Vector2(currentX, currentY), textSize, 0.0f, Vector3::One, mTexMove, Vector4::UnitRect, cameraPos);
        }
        currentY += rowSpacing;
    }

    // Row 3: [Ctrl] "Crouch"
    if (mShowCrouch)
    {
        float currentX = startX - 20.0f;
        if (mTexCtrl)
        {
            renderer->DrawTexture(Vector2(currentX, currentY), ctrlSize, 0.0f, Vector3::One, mTexCtrl, Vector4::UnitRect, cameraPos);
        }
        currentX += 70.0f;

        if (mTexCrouch)
        {
            Vector2 textSize(mTexCrouch->GetWidth() * textScale, mTexCrouch->GetHeight() * textScale);
            renderer->DrawTexture(Vector2(currentX, currentY), textSize, 0.0f, Vector3::One, mTexCrouch, Vector4::UnitRect, cameraPos);
        }
        currentY += rowSpacing;
    }

    // Row 4: [S] [Down] "Dance"
    if (mShowDance)
    {
        float currentX = startX - 40.0f;
        
        if (mTexS) renderer->DrawTexture(Vector2(currentX, currentY), keySize, 0.0f, Vector3::One, mTexS, Vector4::UnitRect, cameraPos);
        currentX += 30.0f;

        if (mTexDown) renderer->DrawTexture(Vector2(currentX, currentY), keySize, 0.0f, Vector3::One, mTexDown, Vector4::UnitRect, cameraPos);
        currentX += 60.0f;

        if (mTexDance)
        {
            Vector2 textSize(mTexDance->GetWidth() * textScale, mTexDance->GetHeight() * textScale);
            renderer->DrawTexture(Vector2(currentX, currentY), textSize, 0.0f, Vector3::One, mTexDance, Vector4::UnitRect, cameraPos);
        }
        currentY += rowSpacing;
    }

    // Row 5: [Mouse] "Shoot"
    if (mShowShoot)
    {
        float currentX = startX - 20.0f;
        if (mTexMouseLeft)
        {
            // Tint mouse red as requested "left button highlighted"
            renderer->DrawTexture(Vector2(currentX, currentY), mouseSize, 0.0f, Vector3(1.0f, 0.5f, 0.5f), mTexMouseLeft, Vector4::UnitRect, cameraPos);
        }
        currentX += 60.0f;

        if (mTexShoot)
        {
            Vector2 textSize(mTexShoot->GetWidth() * textScale, mTexShoot->GetHeight() * textScale);
            renderer->DrawTexture(Vector2(currentX, currentY), textSize, 0.0f, Vector3::One, mTexShoot, Vector4::UnitRect, cameraPos);
        }
        currentY += rowSpacing;
    }

    // Row 6: [Shift] "Run"
    if (mShowRun)
    {
        float currentX = startX - 30.0f;
        if (mTexShift)
        {
            renderer->DrawTexture(Vector2(currentX, currentY), ctrlSize, 0.0f, Vector3::One, mTexShift, Vector4::UnitRect, cameraPos);
        }
        currentX += 70.0f;

        if (mTexRun)
        {
            Vector2 textSize(mTexRun->GetWidth() * textScale, mTexRun->GetHeight() * textScale);
            renderer->DrawTexture(Vector2(currentX, currentY), textSize, 0.0f, Vector3::One, mTexRun, Vector4::UnitRect, cameraPos);
        }
        currentY += rowSpacing;
    }

    // Restore LightShader
    renderer->SetActiveShader(renderer->GetLightShader());
}


