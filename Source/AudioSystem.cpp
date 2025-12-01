#include "AudioSystem.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <filesystem>

SoundHandle SoundHandle::Invalid;

// Create the AudioSystem with specified number of channels
// (Defaults to 8 channels)
AudioSystem::AudioSystem(int numChannels)
{
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_AllocateChannels(numChannels);
    mChannels.resize(numChannels);
}

// Destroy the AudioSystem
AudioSystem::~AudioSystem()
{
    for (auto& sound : mSounds)
    {
        Mix_FreeChunk(sound.second);
    }
    mSounds.clear();

    Mix_CloseAudio();
}

// Updates the status of all the active sounds every frame
void AudioSystem::Update(float deltaTime)
{
    for(int i = 0; i < mChannels.size(); i++)
    {
        if(mChannels[i].IsValid())
        {
            if(!Mix_Playing(i)) {
                mHandleMap.erase(mChannels[i]);
                mChannels[i].Reset();
            }
        }
    }
}

// Plays the sound with the specified name and loops if looping is true
// Returns the SoundHandle which is used to perform any other actions on the
// sound when active
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
SoundHandle AudioSystem::PlaySound(const std::string& soundName, bool looping)
{
    // Get the sound with the given name
    Mix_Chunk *sound = GetSound(soundName);

    if (sound == nullptr) {
        SDL_Log("[AudioSystem] PlaySound couldn't find sound for %s", soundName.c_str());
        return SoundHandle::Invalid;
    }

    int channel = -1;

    // Try find a free channel
    for (int i = 0; i < mChannels.size(); i++) {
        // A free channel is marked by an invalid SoundHandle
        if (!mChannels[i].IsValid()) {
            channel = i;
            break;
        }
    }

    // If no channel is available, apply the replacement policy
    if (channel == -1) {
        // Policy i: Replace the oldest instance *of the same sound
        SoundHandle oldestHandle = SoundHandle::Invalid;
        for (const auto& activeHandle : mChannels) {
            if (mHandleMap.count(activeHandle) && mHandleMap[activeHandle].mSoundName == soundName) {
                if (!oldestHandle.IsValid() || activeHandle < oldestHandle) {
                    oldestHandle = activeHandle;
                }
            }
        }

        if (oldestHandle.IsValid()) {
            // Found an old instance of the same sound to stop
            channel = mHandleMap[oldestHandle].mChannel;
            StopSound(oldestHandle);
        }
        else {
            // Policy ii: Replace the oldest non-looping sound
            for (const auto& activeHandle : mChannels) {
                if (mHandleMap.count(activeHandle) && !mHandleMap[activeHandle].mIsLooping) {
                    if (!oldestHandle.IsValid() || activeHandle < oldestHandle) {
                        oldestHandle = activeHandle;
                    }
                }
            }

            if (oldestHandle.IsValid()) {
                // Found an old non-looping sound to stop
                channel = mHandleMap[oldestHandle].mChannel;
                StopSound(oldestHandle);
            }
            else {
                // Policy iii: Replace the oldest sound (any)
                oldestHandle = mChannels[0];
                for (size_t i = 1; i < mChannels.size(); i++) {
                    if (mChannels[i] < oldestHandle) {
                        oldestHandle = mChannels[i];
                    }
                }
                
                channel = mHandleMap[oldestHandle].mChannel;
                StopSound(oldestHandle);
            }
        }
    }

    // Play the sound on the found channel 
    
    // Increment the handle counter [cite: 594]
    ++mLastHandle;
    
    // Create the handle information
    HandleInfo info;
    info.mSoundName = soundName;
    info.mChannel = channel;
    info.mIsLooping = looping;
    info.mIsPaused = false;

    // Add to the map and the channel vector
    mHandleMap.emplace(mLastHandle, info);
    mChannels[channel] = mLastHandle;

    // Play the sound on SDL_Mixer
    // (SDL_Mixer uses -1 for infinite loops, 0 for "play once")
    int loops = looping ? -1 : 0;
    if (Mix_PlayChannel(channel, sound, loops) == -1) {
        SDL_Log("[AudioSystem] Mix_PlayChannel failed: %s", Mix_GetError());
        // If it fails, clean up what we just added
        mHandleMap.erase(mLastHandle);
        mChannels[channel].Reset();
        return SoundHandle::Invalid;
    }

    return mLastHandle;
}

// Stops the sound if it is currently playing
void AudioSystem::StopSound(SoundHandle sound)
{
    if(mHandleMap.find(sound) == mHandleMap.end())
    {
        SDL_Log("[AudioSystem] StopSound couldn't find handle %s", sound.GetDebugStr());
        return;
    }

    int channel = mHandleMap[sound].mChannel;

    Mix_HaltChannel(channel);
    mHandleMap.erase(sound);
    mChannels[channel].Reset();
}

// Pauses the sound if it is currently playing
void AudioSystem::PauseSound(SoundHandle sound)
{
    if(mHandleMap.find(sound) == mHandleMap.end())
    {
        SDL_Log("[AudioSystem] PauseSound couldn't find handle %s", sound.GetDebugStr());
        return;
    }

    if(!mHandleMap[sound].mIsPaused)
    {
        Mix_Pause(mHandleMap[sound].mChannel);
        mHandleMap[sound].mIsPaused = true;
    }
}

// Resumes the sound if it is currently paused
void AudioSystem::ResumeSound(SoundHandle sound)
{
    if(mHandleMap.find(sound) == mHandleMap.end())
    {
        SDL_Log("[AudioSystem] ResumeSound couldn't find handle %s", sound.GetDebugStr());
        return;
    }

    if(mHandleMap[sound].mIsPaused)
    {
        Mix_Resume(mHandleMap[sound].mChannel);
        mHandleMap[sound].mIsPaused = false;
    }
}

void AudioSystem::PauseAllSounds(SoundHandle exception)
{
    for (auto& handle : mHandleMap) {
        if (handle.first != exception && !handle.second.mIsPaused) {
            Mix_Pause(handle.second.mChannel);
            handle.second.mIsPaused = true;
        }
    }
}

void AudioSystem::ResumeAllSounds(SoundHandle exception)
{
    for (auto& handle : mHandleMap) {
        if (handle.first != exception && handle.second.mIsPaused) {
            Mix_Resume(handle.second.mChannel);
            handle.second.mIsPaused = false;
        }
    }
}

// Returns the current state of the sound
SoundState AudioSystem::GetSoundState(SoundHandle sound)
{
    if(mHandleMap.find(sound) == mHandleMap.end())
    {
        return SoundState::Stopped;
    }

    if(mHandleMap[sound].mIsPaused)
    {
        return SoundState::Paused;
    }

    return SoundState::Playing;
}

// Stops all sounds on all channels
void AudioSystem::StopAllSounds(SoundHandle exception)
{
    for (auto iter = mHandleMap.begin(); iter != mHandleMap.end();) {
        SoundHandle currentHandle = iter->first;
        HandleInfo& info = iter->second;

        if (currentHandle == exception) {
            ++iter;
        }
        else {
            Mix_HaltChannel(info.mChannel);
            mChannels[info.mChannel].Reset();
            iter = mHandleMap.erase(iter);
        }
    }
}

// Cache all sounds under Assets/Sounds
void AudioSystem::CacheAllSounds()
{
#ifndef __clang_analyzer__
    std::error_code ec{};
    for (const auto& rootDirEntry : std::filesystem::directory_iterator{"Assets/Sounds", ec})
    {
        std::string extension = rootDirEntry.path().extension().string();
        if (extension == ".ogg" || extension == ".wav" || extension == ".mp3")
        {
            std::string fileName = rootDirEntry.path().stem().string();
            fileName += extension;
            CacheSound(fileName);
        }
    }
#endif
}

// Used to preload the sound data of a sound
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
void AudioSystem::CacheSound(const std::string& soundName)
{
    GetSound(soundName);
}

// If the sound is already loaded, returns Mix_Chunk from the map.
// Otherwise, will attempt to load the file and save it in the map.
// Returns nullptr if sound is not found.
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
Mix_Chunk* AudioSystem::GetSound(const std::string& soundName)
{
    std::string fileName = "../Assets/Sounds/";
    fileName += soundName;

    Mix_Chunk* chunk = nullptr;
    auto iter = mSounds.find(fileName);
    if (iter != mSounds.end())
    {
        chunk = iter->second;
    }
    else
    {
        chunk = Mix_LoadWAV(fileName.c_str());
        if (!chunk)
        {
            SDL_Log("[AudioSystem] Failed to load sound file %s", fileName.c_str());
            return nullptr;
        }

        mSounds.emplace(fileName, chunk);
    }
    return chunk;
}

// Input for debugging purposes
void AudioSystem::ProcessInput(const Uint8* keyState)
{
    // Debugging code that outputs all active sounds on leading edge of period key
    if (keyState[SDL_SCANCODE_PERIOD] && !mLastDebugKey)
    {
        SDL_Log("[AudioSystem] Active Sounds:");
        for (size_t i = 0; i < mChannels.size(); i++)
        {
            if (mChannels[i].IsValid())
            {
                auto iter = mHandleMap.find(mChannels[i]);
                if (iter != mHandleMap.end())
                {
                    HandleInfo& hi = iter->second;
                    SDL_Log("Channel %d: %s, %s, looping = %d, paused = %d",
                            static_cast<unsigned>(i), mChannels[i].GetDebugStr(),
                            hi.mSoundName.c_str(), hi.mIsLooping, hi.mIsPaused);
                }
                else
                {
                    SDL_Log("Channel %d: %s INVALID", static_cast<unsigned>(i),
                            mChannels[i].GetDebugStr());
                }
            }
        }
    }

    mLastDebugKey = keyState[SDL_SCANCODE_PERIOD];
}
