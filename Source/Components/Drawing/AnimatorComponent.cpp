//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "AnimatorComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include "../../Json.h"
#include "../../Renderer/Texture.h"
#include <cmath>
#include <fstream>

AnimatorComponent::AnimatorComponent(class Actor* owner, const std::string &texPath, const std::string &dataPath,
                                     int width, int height, int drawOrder)
        :DrawComponent(owner,  drawOrder)
        ,mAnimTimer(0.0f)
        ,mIsPaused(false)
        ,mIsLooping(true)
        ,mIsFinished(false)
        ,mWidth(width)
        ,mHeight(height)
    ,mUseFrameAspect(false)
        ,mTextureFactor(1.0f)
        ,mDefaultTexture(nullptr)
        ,mTexturePath(texPath)
        ,mDataPath(dataPath)
{
    if (mOwner && mOwner->GetGame()) {
        mDefaultTexture = mOwner->GetGame()->GetRenderer()->GetTexture(texPath);
    }

    if (!dataPath.empty()) {
        LoadSpriteSheetData(texPath, dataPath);
    }
}

AnimatorComponent::~AnimatorComponent()
{
    mAnimations.clear();
    mSpriteFrames.clear();
}

size_t AnimatorComponent::LoadSpriteSheetData(const std::string& texturePath, const std::string& dataPath)
{
    if (!mOwner || !mOwner->GetGame()) {
        return mSpriteFrames.size();
    }

    auto* texture = mOwner->GetGame()->GetRenderer()->GetTexture(texturePath);
    if (!texture) {
        SDL_Log("Failed to load sprite texture: %s", texturePath.c_str());
        return mSpriteFrames.size();
    }

    if (!mDefaultTexture) {
        mDefaultTexture = texture;
    }

    size_t startIndex = mSpriteFrames.size();

    if (dataPath.empty()) {
        SpriteFrame frame;
        frame.texture = texture;
        frame.texRect = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
        frame.pixelSize = Vector2(static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight()));
        mSpriteFrames.emplace_back(frame);
        return startIndex;
    }

    std::ifstream spriteSheetFile(dataPath);

    if (!spriteSheetFile.is_open()) {
        SDL_Log("Failed to open sprite sheet data file: %s", dataPath.c_str());
        return mSpriteFrames.size();
    }

    nlohmann::json spriteSheetData = nlohmann::json::parse(spriteSheetFile, nullptr, false);

    if (spriteSheetData.is_discarded()) {
        SDL_Log("Failed to parse sprite sheet data file: %s", dataPath.c_str());
        return mSpriteFrames.size();
    }

    auto textureWidth = static_cast<float>(spriteSheetData["meta"]["size"]["w"].get<int>());
    auto textureHeight = static_cast<float>(spriteSheetData["meta"]["size"]["h"].get<int>());

    const auto& framesNode = spriteSheetData["frames"];

    if (framesNode.is_array()) {
        for (const auto& frame : framesNode) {
            AppendFramesFromJson(frame, texture, textureWidth, textureHeight);
        }
    }
    else if (framesNode.is_object()) {
        std::vector<std::string> keys;
        for (auto it = framesNode.begin(); it != framesNode.end(); ++it) {
            keys.push_back(it.key());
        }
        std::sort(keys.begin(), keys.end());

        for (const auto& key : keys) {
            AppendFramesFromJson(framesNode[key], texture, textureWidth, textureHeight);
        }
    }

    return startIndex;
}

void AnimatorComponent::Draw(Renderer* renderer)
{
    if (!mIsVisible) {
        return;
    }

    auto DrawFrame = [&](const std::string& animName, float animTimer, float alpha) {
        Vector4 texRect(0.0f, 0.0f, 1.0f, 1.0f);
        Texture* texture = nullptr;
        const SpriteFrame* activeFrame = nullptr;

        if (!animName.empty() && !mAnimations.empty())
        {
            int frameIndex = static_cast<int>(animTimer);
            const auto& animFrames = mAnimations[animName];
            if (!animFrames.empty()) {
                int spriteIndex = animFrames[frameIndex % animFrames.size()];
                if (spriteIndex >= 0 && spriteIndex < static_cast<int>(mSpriteFrames.size())) {
                    activeFrame = &mSpriteFrames[spriteIndex];
                    texture = activeFrame->texture;
                    texRect = activeFrame->texRect;
                }
            }
        }
        else if (!mSpriteFrames.empty())
        {
            activeFrame = &mSpriteFrames.front();
            texture = activeFrame->texture;
            texRect = activeFrame->texRect;
        }
        else
        {
            texture = mDefaultTexture;
        }

        if (!texture) {
            return;
        }

        Vector2 flipScale(1.0f, 1.0f);
        if (mOwner->GetScale().x < 0.0f) {
            flipScale.x = -1.0f;
        }

        float scaleX = fabsf(mOwner->GetScale().x);
        float scaleY = fabsf(mOwner->GetScale().y);

        Vector2 drawPos = mOwner->GetPosition() + mDrawOffset;

        Vector2 baseSize(
            static_cast<float>(mWidth),
            static_cast<float>(mHeight)
        );

        if (mUseFrameAspect)
        {
            float aspect = 1.0f;
            if (activeFrame && activeFrame->pixelSize.y > 0.0f) {
                aspect = activeFrame->pixelSize.x / activeFrame->pixelSize.y;
            } else if (texture && texture->GetHeight() > 0) {
                aspect = static_cast<float>(texture->GetWidth()) / static_cast<float>(texture->GetHeight());
            }
            
            baseSize.y = static_cast<float>(mHeight);
            baseSize.x = baseSize.y * aspect;
        }

        Vector2 finalSize(
            baseSize.x * (scaleX <= 0.0f ? 1.0f : scaleX),
            baseSize.y * (scaleY <= 0.0f ? 1.0f : scaleY)
        );

        renderer->DrawTexture(
                drawPos,
                finalSize,
                mOwner->GetRotation(),
                Vector3(1.0f, 1.0f, 1.0f),
                texture,
                texRect,
                mOwner->GetGame()->GetCameraPos(),
                flipScale,
                mTextureFactor,
                alpha
        );
    };

    if (mIsTransitioning) {
        float t = mTransitionTimer / mTransitionDuration;
        DrawFrame(mPrevAnimName, mPrevAnimTimer, 1.0f - t);
        DrawFrame(mAnimName, mAnimTimer, t);
    } else {
        DrawFrame(mAnimName, mAnimTimer, 1.0f);
    }
}

void AnimatorComponent::Update(float deltaTime)
{
    if (mIsPaused || mAnimations.empty() || mAnimName.empty()) {
        return;
    }

    // Update animation timer
    mAnimTimer += mAnimFPS * deltaTime;

    if (mIsTransitioning) {
        mTransitionTimer += deltaTime;
        mPrevAnimTimer += mAnimFPS * deltaTime;
        if (mTransitionTimer >= mTransitionDuration) {
            mIsTransitioning = false;
        }
    }

    // Get the number of frames in the current animation
    float numFrames = static_cast<float>(mAnimations[mAnimName].size());

    // Wrap the timer if it exceeds the number of frames
    if (mAnimTimer >= numFrames) {
        if (mIsLooping) {
            mAnimTimer -= numFrames;
        } else {
            mAnimTimer = numFrames - 0.001f;
            mIsFinished = true;
        }
    } else {
        if (!mIsLooping && mAnimTimer < numFrames) {
             mIsFinished = false;
        }
    }
}

void AnimatorComponent::SetAnimation(const std::string& name, float transitionDuration)
{
    // Only change the animation if it's a new one
    if (mAnimName != name) {
        if (transitionDuration > 0.0f && !mAnimName.empty()) {
            mPrevAnimName = mAnimName;
            mPrevAnimTimer = mAnimTimer;
            mIsTransitioning = true;
            mTransitionTimer = 0.0f;
            mTransitionDuration = transitionDuration;
        } else {
            mIsTransitioning = false;
        }

        mAnimName = name;
        // Reset the timer to the start of the new animation
        mAnimTimer = 0.0f;
        mIsFinished = false;
    }
}

int AnimatorComponent::GetCurrentFrameIndex() const
{
    if (mAnimName.empty() || mAnimations.find(mAnimName) == mAnimations.end()) {
        return 0;
    }
    return static_cast<int>(mAnimTimer);
}

int AnimatorComponent::GetCurrentAnimationLength() const
{
    if (mAnimName.empty() || mAnimations.find(mAnimName) == mAnimations.end()) {
        return 0;
    }
    return static_cast<int>(mAnimations.at(mAnimName).size());
}

void AnimatorComponent::AddAnimation(const std::string& name, const std::vector<int>& spriteNums)
{
    mAnimations.emplace(name, spriteNums);
}

size_t AnimatorComponent::AppendSpriteSheet(const std::string& texturePath, const std::string& dataPath)
{
    return LoadSpriteSheetData(texturePath, dataPath);
}

void AnimatorComponent::SetSize(float width, float height)
{
    mWidth = width;
    mHeight = height;
}

void AnimatorComponent::SetUseFrameAspect(bool enable)
{
    mUseFrameAspect = enable;
}

void AnimatorComponent::SetOffset(const Vector2& offset)
{
    mDrawOffset = offset;
}

void AnimatorComponent::SetSpriteSheet(const std::string& texturePath, const std::string& dataPath)
{
    if (!mOwner || !mOwner->GetGame()) return;

    mDefaultTexture = mOwner->GetGame()->GetRenderer()->GetTexture(texturePath);
    mAnimName.clear();
    mAnimations.clear();
    ClearSpriteData();

    if (!dataPath.empty()) {
        LoadSpriteSheetData(texturePath, dataPath);
    }
}

void AnimatorComponent::ClearSpriteData()
{
    mSpriteFrames.clear();
}

void AnimatorComponent::AppendFramesFromJson(const nlohmann::json& frameNode, Texture* texture,
                                            float textureWidth, float textureHeight)
{
    if (!frameNode.contains("frame")) {
        return;
    }

    const auto& frame = frameNode["frame"];
    int x = frame["x"].get<int>();
    int y = frame["y"].get<int>();
    int w = frame["w"].get<int>();
    int h = frame["h"].get<int>();

    if (w <= 0 || h <= 0) {
        return;
    }

    SpriteFrame spriteFrame{};
    spriteFrame.texture = texture;
    spriteFrame.texRect = Vector4(
            static_cast<float>(x) / textureWidth,
            static_cast<float>(y) / textureHeight,
            static_cast<float>(w) / textureWidth,
            static_cast<float>(h) / textureHeight);
    spriteFrame.pixelSize = Vector2(static_cast<float>(w), static_cast<float>(h));

    mSpriteFrames.emplace_back(spriteFrame);
}