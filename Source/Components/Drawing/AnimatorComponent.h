//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once

#include <unordered_map>
#include <vector>
#include "DrawComponent.h"
#include "../../Json.h"

class AnimatorComponent : public DrawComponent {
public:
    // (Lower draw order corresponds with further back)
    AnimatorComponent(class Actor* owner, const std::string &texturePath, const std::string &dataPath,
            int width, int height, int drawOrder = 100);
    ~AnimatorComponent() override;

    void Draw(Renderer* renderer) override;
    void Update(float deltaTime) override;

    // Use to change the FPS of the animation
    void SetAnimFPS(float fps) { mAnimFPS = fps; }

    // Set the current active animation
    void SetAnimation(const std::string& name, float transitionDuration = 0.0f);

    const std::string& GetAnimName() const { return mAnimName; }

    // Use to pause/unpause the animation
    void SetIsPaused(bool pause) { mIsPaused = pause; }

    // Add an animation of the corresponding name to the animation map
    void AddAnimation(const std::string& name, const std::vector<int>& images);

    // Append an extra sprite sheet to this animator and get the starting frame index
    size_t AppendSpriteSheet(const std::string& texturePath, const std::string& dataPath);

    // Useful helper when configuring animations
    size_t GetFrameCount() const { return mSpriteFrames.size(); }

    int GetCurrentFrameIndex() const;
    int GetCurrentAnimationLength() const;

    void SetLooping(bool looping) { mIsLooping = looping; }
    bool IsFinished() const { return mIsFinished; }

    // Set size
    void SetSize(float width, float height);

    const std::string& GetTexturePath() const { return mTexturePath; }
    const std::string& GetDataPath() const { return mDataPath; }

    // Match draw width to each frame's native aspect ratio when enabled
    void SetUseFrameAspect(bool enable);

    // Set offset for drawing
    void SetOffset(const Vector2& offset); 

    // Set sprite sheet (texture and data)
    void SetSpriteSheet(const std::string& texturePath, const std::string& dataPath);

private:
    struct SpriteFrame {
        class Texture* texture;
        Vector4 texRect;
        Vector2 pixelSize;
    };

    size_t LoadSpriteSheetData(const std::string& texturePath, const std::string& dataPath);
    void ClearSpriteData();
    void AppendFramesFromJson(const nlohmann::json& frameNode, class Texture* texture,
                              float textureWidth, float textureHeight);

    // Sprite sheet texture
    class Texture* mDefaultTexture;

    // Vector of frames (each knows which texture it references)
    std::vector<SpriteFrame> mSpriteFrames;

    // Map of animation name to vector of textures corresponding to the animation
    std::unordered_map<std::string, std::vector<int>> mAnimations;

    // Name of current animation
    std::string mAnimName;

    // Tracks current elapsed time in animation
    float mAnimTimer = 0.0f;

    // The frames per second the animation should run at
    float mAnimFPS = 10.0f;

    // Whether or not the animation is paused (defaults to false)
    bool mIsPaused = false;
    bool mIsLooping = true;
    bool mIsFinished = false;

    // Size
    int mWidth;
    int mHeight;
    bool mUseFrameAspect = false;

    // Offset for drawing
    Vector2 mDrawOffset = Vector2::Zero;

    // Transition
    std::string mPrevAnimName;
    float mPrevAnimTimer = 0.0f;
    float mTransitionTimer = 0.0f;
    float mTransitionDuration = 0.0f;
    bool mIsTransitioning = false;

    float mTextureFactor;

    std::string mTexturePath;
    std::string mDataPath;
};

