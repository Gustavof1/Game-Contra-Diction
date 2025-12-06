#include "Spaceman.h"
#include "SpacemanArm.h"
#include "PlayerBullet.h"
#include "PolicemanBullet.h"
#include "EnemyLaser.h"
#include "Laser.h"
#include "GasCloud.h"
#include "Block.h"
#include "Goomba.h"
#include "Mushroom.h"
#include "Coin.h"
#include "../Game.h"
#include "../Math.h"
#include "../Random.h"
#include "../Components/Drawing/AnimatorComponent.h"
#include "../Components/Physics/RigidBodyComponent.h"
#include "../Components/Physics/AABBColliderComponent.h"
#include "../Components/ParticleSystemComponent.h"
#include <SDL.h>
#include <algorithm>
#include <vector>
#include <set>

namespace
{
    std::vector<int> MakeFrameRange(size_t start, size_t count)
    {
        std::vector<int> frames;
        frames.reserve(count);
        for (size_t i = 0; i < count; ++i)
        {
            frames.emplace_back(static_cast<int>(start + i));
        }
        return frames;
    }

    std::vector<int> MakePingPongFrameRange(size_t start, size_t count)
    {
        std::vector<int> frames;
        if (count == 0) return frames;
        
        frames.reserve(count * 2 - 2);
        for (size_t i = 0; i < count; ++i)
        {
            frames.emplace_back(static_cast<int>(start + i));
        }
        for (int i = static_cast<int>(count) - 2; i > 0; --i)
        {
            frames.emplace_back(static_cast<int>(start + i));
        }
        return frames;
    }

    constexpr float kSpriteHeightMultiplier = 2.0f;
    constexpr float kJumpHeightMultiplier = 2.5f;
    constexpr float kCrouchWalkHeightMultiplier = 1.8f;
    constexpr float kWalkAspectRatio = 77.0f / 128.0f;
    constexpr float kSpriteWidthMultiplier = kSpriteHeightMultiplier * kWalkAspectRatio;
    constexpr float kColliderWidthMultiplier = 0.6f;
    constexpr float kColliderHeightMultiplier = 2.0f;
    constexpr float kColliderYOffsetMultiplier = 0.0f;
}

Spaceman::Spaceman(Game* game)
    : Actor(game)
    , mMoveForce(7000.0f)
    , mJumpImpulse(-700.0f)
    , mShootCooldown(0.25f)
    , mShootTimer(0.0f)
    , mShootAnimTimer(0.0f)
    , mIsRunning(false)
    , mIsDead(false)
    , mIsShooting(false)
    , mRigidBodyComponent(nullptr)
    , mAnimatorComponent(nullptr)
    , mColliderComponent(nullptr)
    , mJumpCount(0)
    , mWPressedLastFrame(false)
    , mSPressedLastFrame(false)
    , mPosture(PlayerPosture::Standing)
    , mArm(nullptr)
    , mLaser(nullptr)
    , mShootDirection(1.0f, 0.0f)
    , mTabPressedLastFrame(false)
    , mIsFiringLaser(false)
    , mIsCrouchingHeld(false)
    , mIsDancing(false)
    , mDeathTimer(0.0f)
    , mHeadItem(ItemType::None)
    , mHandItem(ItemType::None)
{
    mArm = new SpacemanArm(game);
    mArm->SetPosition(GetPosition());
    mArm->SetVisible(false); // Default hidden until weapon equipped

    mLaser = new Laser(game);
    mLaser->SetActive(false);

    mBulletParticleSystem = new ParticleSystemComponent(this, 48, 48, 
        "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.png",
        "../Assets/Sprites/ObjectsScenery-ContraDiction/bulletparticle.json",
        20,
        10,
        ColliderLayer::PlayerProjectile,
        false, // No gravity
        0.25f, // Small collider (48 * 0.25 = 12px)
        Vector2(12.0f, 12.0f) // Shift sprite to bottom-right (scaled for 48x48)
        );
    mBulletParticleSystem->SetCollisionCallback([](AABBColliderComponent* other) {
        if (other->GetLayer() == ColliderLayer::Enemy)
        {
            other->GetOwner()->Kill();
        }
    });

    auto* animator = new AnimatorComponent(
        this,
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_walk_walking128x128.png",
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_walk_walking128x128.json",
        128,
        128);

    animator->SetSize(Game::TILE_SIZE * kSpriteWidthMultiplier, Game::TILE_SIZE * kSpriteHeightMultiplier);
    animator->SetUseFrameAspect(true);

    size_t walkCount = animator->GetFrameCount();
    auto walkFrames = MakeFrameRange(0, walkCount);
    if (walkFrames.empty() && animator->GetFrameCount() > 0)
    {
        walkFrames.emplace_back(0);
    }

    const size_t jumpStart = animator->AppendSpriteSheet(
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_side_jump_jumpeconomic.png",
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_side_jump_jumpeconomic.json");
    size_t jumpFrameCount = animator->GetFrameCount() - jumpStart;

    // Removed shooting animation loading

    const int idleFrame = walkFrames.empty() ? 0 : walkFrames.front();
    animator->AddAnimation("idle", { idleFrame });
    animator->AddAnimation("run", walkFrames);

    if (jumpFrameCount > 7)
    {
        animator->AddAnimation("jump", MakeFrameRange(jumpStart + 5, jumpFrameCount - 5));
        animator->AddAnimation("double_jump", MakeFrameRange(jumpStart + 5, jumpFrameCount - 5));
    }
    else if (jumpFrameCount > 0)
    {
        animator->AddAnimation("jump", MakeFrameRange(jumpStart, jumpFrameCount));
        animator->AddAnimation("double_jump", MakeFrameRange(jumpStart, jumpFrameCount));
    }
    else
    {
        animator->AddAnimation("jump", { idleFrame });
        animator->AddAnimation("double_jump", { idleFrame });
    }

    const size_t crouchStart = animator->AppendSpriteSheet(
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_crouching_crouching.png",
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_crouching_crouching.json");
    size_t crouchFrameCount = animator->GetFrameCount() - crouchStart;
    if (crouchFrameCount > 0) {
        animator->AddAnimation("crouch", MakeFrameRange(crouchStart, crouchFrameCount));
    }

    const size_t crouchWalkStart = animator->AppendSpriteSheet(
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_crouch_walking_crouch_walking.png",
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_crouch_walking_crouch_walking.json");
    size_t crouchWalkFrameCount = animator->GetFrameCount() - crouchWalkStart;
    if (crouchWalkFrameCount > 0) {
        animator->AddAnimation("crouch_walk", MakeFrameRange(crouchWalkStart, crouchWalkFrameCount));
    }

    const size_t danceStart = animator->AppendSpriteSheet(
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_dancing_dancing.png",
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_dancing_dancing.json");
    size_t danceFrameCount = animator->GetFrameCount() - danceStart;
    if (danceFrameCount > 0) {
        animator->AddAnimation("dance", MakePingPongFrameRange(danceStart, danceFrameCount));
    }

    const size_t deathStart = animator->AppendSpriteSheet(
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_death_default.png",
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_death_default.json");
    size_t deathFrameCount = animator->GetFrameCount() - deathStart;
    if (deathFrameCount > 0) {
        animator->AddAnimation("death", MakeFrameRange(deathStart, deathFrameCount));
    }

    {
        animator->AddAnimation("shoot", { idleFrame });
    }
    animator->SetAnimation("idle");
    animator->SetAnimFPS(18.0f);
    mAnimatorComponent = animator;

    // Peace Animator (Combined Spritesheet)
    mPeaceAnimator = new AnimatorComponent(this, 
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_combined.png",
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_combined.json",
        128, 128);
    
    // Ranges from JSON:
    // crouch walking: 0-13 (14 frames)
    mPeaceAnimator->AddAnimation("crouch_walk", MakeFrameRange(0, 14));
    // crouching: 14-29 (16 frames)
    mPeaceAnimator->AddAnimation("crouch", MakeFrameRange(14, 16));
    // dancing: 30-61 (32 frames)
    mPeaceAnimator->AddAnimation("dance", MakePingPongFrameRange(30, 32));
    // dashing: 62-88 (27 frames) - Using as dash
    // Start from middle (approx frame 13 of 27) -> 62 + 13 = 75
    // Count = 27 - 13 = 14
    mPeaceAnimator->AddAnimation("dash", MakeFrameRange(75, 14));
    
    // walk: 89-136 (48 frames)
    mPeaceAnimator->AddAnimation("run", MakeFrameRange(89, 48));
    mPeaceAnimator->AddAnimation("idle", { 89 });
    mPeaceAnimator->AddAnimation("death", { 14 }); 

    // Load separate jump animation for peace mode
    const size_t peaceJumpStart = mPeaceAnimator->AppendSpriteSheet(
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_side_jump_no_weapon.png",
        "../Assets/Sprites/Spaceman-ContraDiction/space_dandy_side_jump_no_weapon.json");
    size_t peaceJumpCount = mPeaceAnimator->GetFrameCount() - peaceJumpStart;
    
    if (peaceJumpCount > 0) {
        mPeaceAnimator->AddAnimation("jump", MakeFrameRange(peaceJumpStart, peaceJumpCount));
        mPeaceAnimator->AddAnimation("double_jump", MakeFrameRange(peaceJumpStart, peaceJumpCount));
    } else {
        // Fallback
        mPeaceAnimator->AddAnimation("jump", { 89 });
        mPeaceAnimator->AddAnimation("double_jump", { 89 });
    }
    
    mPeaceAnimator->SetVisible(false);

    mRigidBodyComponent = new RigidBodyComponent(this, 1.2f, 35.0f, true);
    const int colliderWidth = static_cast<int>(Game::TILE_SIZE * kColliderWidthMultiplier);
    const int colliderHeight = static_cast<int>(Game::TILE_SIZE * kColliderHeightMultiplier);
    const int colliderOffsetY = static_cast<int>(Game::TILE_SIZE * kColliderYOffsetMultiplier);
    mColliderComponent = new AABBColliderComponent(this, 0, colliderOffsetY, colliderWidth, colliderHeight, ColliderLayer::Player);

    // Initialize Equipment
    EquipHead(ItemType::Headphones);
    EquipHand(ItemType::None);
}

void Spaceman::OnProcessInput(const Uint8* state)
{
    if (mIsDead)  {
        return;
    }

    mIsRunning = false;

    float currentForce = 0.0f;
    if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) {
        currentForce = mMoveForce * 1.8;
    }
    else {
        currentForce = mMoveForce;
    }
    

    float direction = 0.0f;
    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) {
        direction += 1.0f;
    }
    if (state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT]) {
        direction -= 1.0f;
    }

    if (!Math::NearlyZero(direction)) {
        mRigidBodyComponent->ApplyForce(Vector2(direction * currentForce, 0.0f));
        mScale.x = direction > 0.0f ? 1.0f : -1.0f;
        mIsRunning = true;
    }

    bool sPressed = state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN];
    bool wPressed = state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_SPACE];
    bool ctrlPressed = state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL];

    mIsDancing = sPressed;
    bool isActuallyDancing = mIsDancing && mIsOnGround && !mIsRunning && mPosture == PlayerPosture::Standing;

    // Crouch Logic (CTRL)
    if (ctrlPressed) {
        if (mPosture != PlayerPosture::Crouching) {
            SetPosture(PlayerPosture::Crouching);
        }
    } else {
        if (mPosture == PlayerPosture::Crouching) {
            SetPosture(PlayerPosture::Standing);
        }
    }

    // Jump Logic (W)
    if (wPressed && !mWPressedLastFrame)
    {
        if (mPosture == PlayerPosture::Standing) {
            if (mIsOnGround || mJumpCount < 2) {
                PerformJump();
            }
        }
    }

    mSPressedLastFrame = sPressed;
    mWPressedLastFrame = wPressed;
    
    // Arm Rotation & Shooting
    int mouseX, mouseY;
    Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
    Vector2 mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));
    
    Vector2 aimDir = Vector2::Zero;

    // Use Mouse
    Vector2 cameraPos = GetGame()->GetCameraPos();
    Vector2 playerScreenPos = GetPosition() - cameraPos;
    aimDir = mousePos - playerScreenPos;
    aimDir.Normalize();

    mShootDirection = aimDir;
    float angle = std::atan2(aimDir.y, aimDir.x);
    
    if (mArm) {
        if (isActuallyDancing) {
            mArm->SetState(ActorState::Paused);
            mArm->SetVisible(false);
        } else {
            mArm->SetState(ActorState::Active);
            // Only show arm if we have a gun equipped
            bool hasGun = (mHandItem == ItemType::Pistol || mHandItem == ItemType::AlienPistol);
            mArm->SetVisible(hasGun);
            
            bool facingRight = mScale.x > 0.0f;
            bool aimingLeft = (Math::Abs(angle) > Math::PiOver2);
            bool aimingRight = !aimingLeft;
            
            mArm->SetFacingRight(facingRight);

            bool shouldBend = false;
            if (facingRight && aimingLeft) {
                shouldBend = true;
            } else if (!facingRight && aimingRight) {
                shouldBend = true;
            }
            
            mArm->SetBent(shouldBend);

            // Adjust rotation based on sprite direction
            if (facingRight) {
                mArm->SetRotation(angle);
            } else {
                mArm->SetRotation(angle - Math::Pi);
            }
            
            mArm->SetScale(Vector2(1.0f, 1.0f));
        }
    }

    // Weapon Switching - REMOVED for Inventory System
    /*
    if (state[SDL_SCANCODE_TAB] && !mTabPressedLastFrame) {
        if (mArm) {
            WeaponType current = mArm->GetWeaponType();
            mArm->SetWeaponType(current == WeaponType::Pistol ? WeaponType::AlienGun : WeaponType::Pistol);
        }
    }
    mTabPressedLastFrame = state[SDL_SCANCODE_TAB];
    */

    if (!isActuallyDancing && (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))) {
        TryShoot();
    } else {
        StopShoot();
    }

    mIsCrouchingHeld = state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL];
}

void Spaceman::PerformJump()
{
    auto velocity = mRigidBodyComponent->GetVelocity();

    if (mJumpCount == 0)
    {
        GetGame()->GetAudio()->PlaySound("JumpContradiction.wav");
        velocity.y = mJumpImpulse;
    }
    else
    {
        GetGame()->GetAudio()->PlaySound("JumpSuper.wav");
        velocity.y = mJumpImpulse * 0.75f; 
    }

    mRigidBodyComponent->SetVelocity(velocity);
    
    SetOffGround();
    
    mJumpCount++;
}

void Spaceman::SetPosture(PlayerPosture posture)
{
    mPosture = posture;

    float originalW = Game::TILE_SIZE * kColliderWidthMultiplier;
    float originalH = Game::TILE_SIZE * kColliderHeightMultiplier;
    
    float w = originalW;
    float h = originalH;
    
    float offY = 0.0f;

    switch (posture)
    {
        case PlayerPosture::Standing:
            break;
            
        case PlayerPosture::Crouching:
            h = originalH * 0.5f;
            offY = originalH * 0.25f;
            break;
            
        case PlayerPosture::Prone:
            h = originalH * 0.25f;
            w = originalH;
            offY = originalH * 0.375f;
            break;
    }

    if (mColliderComponent)
    {
        mColliderComponent->SetSize(w, h);
        mColliderComponent->SetOffset(Vector2(0.0f, offY));
    }
}

void Spaceman::OnUpdate(float deltaTime)
{
    if (mIsDead)
    {
        mDeathTimer -= deltaTime;
        if (mDeathTimer <= 0.0f)
        {
            GetGame()->SetScene(GameScene::GameOver);
        }
        ManageAnimations();
        return;
    }

    if (mShootTimer > 0.0f)
    {
        mShootTimer -= deltaTime;
    }

    if (mShootAnimTimer > 0.0f)
    {
        mShootAnimTimer -= deltaTime;
        if (mShootAnimTimer <= 0.0f)
        {
            mIsShooting = false;
        }
    }

    if (mRigidBodyComponent && !Math::NearlyZero(mRigidBodyComponent->GetVelocity().y))
    {
        mIsOnGround = false;
    }

    if (mPosition.y > Game::WINDOW_HEIGHT + Game::TILE_SIZE)
    {
        Kill();
        GetGame()->SetScene(GameScene::GameOver);
        return;
    }

    if (mPosition.x < Game::TILE_SIZE * 0.5f)
    {
        mPosition.x = Game::TILE_SIZE * 0.5f;
    }

    if (mArm) {
        // Calculate arm position so it rotates around its pivot
        float angle = mArm->GetRotation();
        bool isBent = mArm->IsBent();
        bool facingRight = mArm->IsFacingRight();
        WeaponType weapon = mArm->GetWeaponType();

        // Shoulder position relative to player center
        // Mirror X offset based on facing direction
        float shoulderX = -3.0f;
        if (mScale.x < 0.0f) {
            shoulderX = 3.0f;
        }
        
        float shoulderY = -8.0f;

        if (mAnimatorComponent && mAnimatorComponent->GetAnimName() == "crouch") {
            int frame = mAnimatorComponent->GetCurrentFrameIndex();
            int totalFrames = mAnimatorComponent->GetCurrentAnimationLength();
            int middleFrame = 11;
            float maxOffset = 25.0f;

            float t = 0.0f;
            if (frame <= middleFrame) {
                t = static_cast<float>(frame) / static_cast<float>(middleFrame);
            } else {
                int framesLeft = totalFrames - 1 - middleFrame;
                if (framesLeft > 0) {
                    t = 1.0f - static_cast<float>(frame - middleFrame) / static_cast<float>(framesLeft);
                }
            }
            shoulderY += maxOffset * t;
        }
        
        Vector2 shoulderOffset(shoulderX, shoulderY); 
        Vector2 shoulderPos = GetPosition() + shoulderOffset;

        // Calculate Offset from Pivot to Center in Local Space
        Vector2 pivotToCenter;
        
        float w = 0.0f;
        float h = 0.0f;
        float scale = 0.5f;

        if (weapon == WeaponType::Pistol) {
            w = (isBent ? 70.0f : 96.0f) * scale;
            h = (isBent ? 44.0f : 25.0f) * scale;
        } else {
            w = (isBent ? 70.0f : 96.0f) * scale;
            h = (isBent ? 44.0f : 31.0f) * scale;
        }
        
        float halfW = w / 2.0f;
        // Pivot is vertically centered
        
        // Adjust pivot for bent arm to be closer to shoulder
        float bentXOffset = 0.0f;
        if (isBent) {
            bentXOffset = 10.0f * scale;
        }

        if (facingRight) {
             // Facing Right: Pivot Left-Center (0, h/2)
             // Center is at (halfW, halfH) relative to Top-Left
             // Vector Pivot -> Center = (halfW, 0)
             pivotToCenter = Vector2(halfW - bentXOffset, 0.0f);
        } else {
             // Facing Left: Pivot Right-Center (W, h/2)
             // Center is at (halfW, halfH) relative to Top-Left
             // Vector Pivot -> Center = (-halfW, 0)
             pivotToCenter = Vector2(-halfW + bentXOffset, 0.0f);
        }
        
        // Rotate pivotToCenter by angle
        float c = Math::Cos(angle);
        float s = Math::Sin(angle);
        
        Vector2 rotatedOffset(
            pivotToCenter.x * c - pivotToCenter.y * s,
            pivotToCenter.x * s + pivotToCenter.y * c
        );
        
        mArm->SetPosition(shoulderPos + rotatedOffset);
    }

    if (mIsFiringLaser && mLaser) {
        // Update laser position
        // Start at arm end? Or shoulder?
        // Let's start at shoulder + direction * offset
        Vector2 startPos = GetPosition() + Vector2(0.0f, -10.0f); // Approx shoulder height
        mLaser->UpdateBeam(startPos, mShootDirection);
    }

    // Flashlight Logic
    if (mHandItem == ItemType::Flashlight) {
        GetGame()->GetRenderer()->SetFlashlightUniforms(true, GetPosition(), mShootDirection);
    } else {
        GetGame()->GetRenderer()->SetFlashlightUniforms(false, Vector2::Zero, Vector2::Zero);
    }

    UpdateGasLogic(deltaTime);

    ManageAnimations();
}

void Spaceman::ManageAnimations()
{
    bool usePeace = (mHandItem == ItemType::None || mHandItem == ItemType::Flashlight);
    
    // If dead, force combat animator because it has the death animation
    if (mIsDead) {
        usePeace = false;
    }

    mAnimatorComponent->SetVisible(!usePeace);
    mPeaceAnimator->SetVisible(usePeace);

    AnimatorComponent* activeAnim = usePeace ? mPeaceAnimator : mAnimatorComponent;

    if (!activeAnim) {
        return;
    }

    if (mIsDead) {
        activeAnim->SetAnimation("death");
        activeAnim->SetLooping(false);
        activeAnim->SetIsPaused(false);
        return;
    }

    activeAnim->SetAnimFPS(18.0f);
    activeAnim->SetOffset(Vector2::Zero);
    activeAnim->SetIsPaused(false);

    // Handle Crouch Stand up transition
    if (mPosture != PlayerPosture::Crouching && activeAnim->GetAnimName() == "crouch") {
         activeAnim->SetLooping(false);
         activeAnim->SetIsPaused(false);
         if (!activeAnim->IsFinished()) {
             return;
         }
    }

    activeAnim->SetLooping(true);
    activeAnim->SetSize(Game::TILE_SIZE * kSpriteWidthMultiplier, Game::TILE_SIZE * kSpriteHeightMultiplier);
    activeAnim->SetUseFrameAspect(true);

    if (mIsDancing && mIsOnGround && !mIsRunning && mPosture == PlayerPosture::Standing) {
        activeAnim->SetAnimation("dance", 0.0f);
        activeAnim->SetAnimFPS(18.0f);
        return;
    }

    if (mPosture == PlayerPosture::Prone) {
        activeAnim->SetAnimation("idle", 0.0f); 
        return;

    }
    
    if (mPosture == PlayerPosture::Crouching) {
        if (mIsRunning) {
            activeAnim->SetAnimation("crouch_walk", 0.0f);
            activeAnim->SetSize(Game::TILE_SIZE * kSpriteWidthMultiplier, Game::TILE_SIZE * kCrouchWalkHeightMultiplier);
            
            float offset = (Game::TILE_SIZE * kSpriteHeightMultiplier - Game::TILE_SIZE * kCrouchWalkHeightMultiplier) / 2.0f;
            activeAnim->SetOffset(Vector2(0.0f, offset));

            activeAnim->SetLooping(true);
            activeAnim->SetIsPaused(false);
        } else {
            activeAnim->SetAnimation("crouch", 0.0f);
            activeAnim->SetLooping(false);
            
            int currentFrame = activeAnim->GetCurrentFrameIndex();
            int middleFrame = 11; 
            // Adjust middle frame logic for peace animator if needed
            if (usePeace) middleFrame = 8; // Guess

            if (currentFrame == middleFrame) {
                 activeAnim->SetIsPaused(true);
            }
        }
        return;
    }

    if (!mIsOnGround) {
        if (mJumpCount >= 2) {
            activeAnim->SetAnimation("double_jump", 0.0f);
        } else {
            activeAnim->SetAnimation("jump", 0.0f);
        }
        activeAnim->SetSize(Game::TILE_SIZE * kSpriteWidthMultiplier, Game::TILE_SIZE * kJumpHeightMultiplier);
        activeAnim->SetAnimFPS(24.0f);
        return;
    }

    if (mIsRunning) {
        const Uint8* state = SDL_GetKeyboardState(nullptr);
        if (usePeace && (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT])) {
            activeAnim->SetAnimation("dash", 0.0f);
        } else {
            activeAnim->SetAnimation("run", 0.0f);
        }
        return;
    }

    if (mIsShooting && !usePeace) {
        activeAnim->SetAnimation("shoot", 0.0f);
        return;
    }

    activeAnim->SetAnimation("idle", 0.0f);
}

void Spaceman::TryShoot()
{
    if (mHandItem != ItemType::Pistol && mHandItem != ItemType::AlienPistol) {
        return;
    }

    WeaponType weapon = mArm ? mArm->GetWeaponType() : WeaponType::Pistol;

    if (weapon == WeaponType::AlienGun) {
        if (mShootTimer > 0.0f) return;

        // Gas Particle System
        // Spawn GasCloud with cone spread
        Vector2 spawnPos = GetPosition() + mShootDirection * 40.0f;
        
        // Add random spread to direction (+- 15 degrees)
        float angle = Math::Atan2(mShootDirection.y, mShootDirection.x);
        float spread = Math::ToRadians(15.0f);
        float randomAngle = Random::GetFloatRange(-spread, spread);
        float finalAngle = angle + randomAngle;
        
        Vector2 spreadDir(Math::Cos(finalAngle), Math::Sin(finalAngle));
        
        GasCloud* gas = new GasCloud(GetGame(), spreadDir);
        gas->SetPosition(spawnPos);
        
        // Play sound occasionally or loop?
        // For now, play sound every shot (might be too frequent if 0.05s)
        // Let's use 0.05s for denser cloud
        mShootTimer = 0.05f;
        return;
    }

    // Pistol Logic
    if (mShootTimer > 0.0f)
    {
        return;
    }

    GetGame()->GetAudio()->PlaySound("Shoot.wav");

    float scaleX = GetScale().x;
    Vector2 offset = (mShootDirection * 40.0f) * (1.0f / scaleX);
    mBulletParticleSystem->EmitParticle(1.2f, 1600.0f, mShootDirection, offset);

    mIsShooting = true;
    mShootAnimTimer = 0.25f;
    mShootTimer = mShootCooldown;
}

void Spaceman::StopShoot()
{
    if (mIsFiringLaser) {
        mIsFiringLaser = false;
        if (mLaser) mLaser->SetActive(false);
    }
}

void Spaceman::UpdateGasLogic(float deltaTime)
{
    // Deprecated: Gas logic is now handled by individual GasCloud collisions
    // This function is kept empty to satisfy interface or can be removed if header is updated.
    // We rely on GasCloud::OnHorizontalCollision calling Actor::ApplyGasExposure
}

void Spaceman::OnHorizontalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (mIsDead) return;

    if (!other)
    {
        return;
    }

    if (other->GetLayer() == ColliderLayer::Hazard) {
        GetGame()->SetGameOverInfo(other->GetOwner());
        Kill();
        return;
    }

    if (other->GetLayer() == ColliderLayer::Enemy)
    {
        if (auto* goomba = dynamic_cast<Goomba*>(other->GetOwner()))
        {
            if (mRigidBodyComponent && mRigidBodyComponent->GetVelocity().y > 0.0f)
            {
                goomba->setStomped(true);
                other->GetOwner()->Kill();
                auto vel = mRigidBodyComponent->GetVelocity();
                vel.y = mJumpImpulse * 0.5f;
                mRigidBodyComponent->SetVelocity(vel);
                SetOffGround();
                return;
            }
        }

        Actor* killer = other->GetOwner();
        if (auto* bullet = dynamic_cast<PolicemanBullet*>(killer)) {
            if (bullet->GetShooter()) {
                killer = bullet->GetShooter();
            }
        } else if (auto* laser = dynamic_cast<EnemyLaser*>(killer)) {
            if (laser->GetShooter()) {
                killer = laser->GetShooter();
            }
        }
        GetGame()->SetGameOverInfo(killer);
        Kill();
    }

    if (other->GetLayer() == ColliderLayer::Collectable)
    {
        if (dynamic_cast<Mushroom*>(other->GetOwner()))
        {
            GetGame()->GetAudio()->PlaySound("PowerUp.wav");
            PowerUp();
            other->GetOwner()->Kill();
        }
        else if (dynamic_cast<Coin*>(other->GetOwner()))
        {
            GetGame()->GetAudio()->PlaySound("Coin.wav");
            GetGame()->AddCoin();
            other->GetOwner()->SetState(ActorState::Destroy);
        }
    }
}

void Spaceman::OnVerticalCollision(const float minOverlap, AABBColliderComponent* other)
{
    if (mIsDead) return;

    if (!other)
    {
        return;
    }

    if (other->GetLayer() == ColliderLayer::Hazard) {
        GetGame()->SetGameOverInfo(other->GetOwner());
        Kill();
        return;
    }

    if (minOverlap > 0.0f)
    {
        mIsOnGround = true;
        mJumpCount = 0;

        if (other->GetLayer() == ColliderLayer::Enemy)
        {
            if (auto* goomba = dynamic_cast<Goomba*>(other->GetOwner()))
            {
                GetGame()->GetAudio()->PlaySound("Stomp.wav");
                goomba->setStomped(true);
            }
            other->GetOwner()->Kill();

            if (mRigidBodyComponent)
            {
                auto vel = mRigidBodyComponent->GetVelocity();
                vel.y = mJumpImpulse * 0.5f;
                mRigidBodyComponent->SetVelocity(vel);
                SetOffGround();
                mJumpCount = 1;
            }
        }
    }
    else if (minOverlap < 0.0f && other->GetLayer() == ColliderLayer::Blocks)
    {
        if (auto* block = dynamic_cast<Block*>(other->GetOwner()))
        {
            GetGame()->GetAudio()->PlaySound("Bump.wav");
            block->Bump();
        }
    }
}

void Spaceman::Kill()
{
    if (mIsDead)
    {
        return;
    }

    if (GetGame()->IsImmortal())
    {
        return;
    }

    GetGame()->GetAudio()->PlaySound("Street Fighter II-DeathSound.mp3");

    mIsDead = true;
    mDeathTimer = 2.0f;

    if (mColliderComponent)
    {
        mColliderComponent->SetLayer(ColliderLayer::Collectable);
    }

    if (mRigidBodyComponent)
    {
        auto vel = mRigidBodyComponent->GetVelocity();
        vel.x = 0.0f;
        mRigidBodyComponent->SetVelocity(vel);
    }

    if (mArm)
    {
        mArm->SetVisible(false);
    }

    GetGame()->SetPlayerIsDead(true);
}

void Spaceman::PowerUp()
{
    mShootCooldown = 0.15f;
}

void Spaceman::PowerDown()
{
    mShootCooldown = 0.25f;
}

void Spaceman::EquipHead(ItemType item)
{
    mHeadItem = item;
    
    // Only update audio if we are in gameplay (not paused/inventory)
    if (GetGame()->GetState() == GameState::Gameplay) {
        if (mHeadItem == ItemType::Headphones) {
            GetGame()->GetAudio()->ResumeSound(GetGame()->GetMusicHandle());
        } else {
            GetGame()->GetAudio()->PauseSound(GetGame()->GetMusicHandle());
        }
    }
}

void Spaceman::EquipHand(ItemType item)
{
    mHandItem = item;
    
    if (mHandItem == ItemType::Pistol) {
        mArm->SetVisible(true);
        mArm->SetWeaponType(WeaponType::Pistol);
    } else if (mHandItem == ItemType::AlienPistol) {
        mArm->SetVisible(true);
        mArm->SetWeaponType(WeaponType::AlienGun);
    } else {
        mArm->SetVisible(false);
    }
}
