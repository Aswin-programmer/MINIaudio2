#pragma once

#include <iostream>
#include "AudioEngine.h"
#include <algorithm>
#include <string>
#include <functional>

// On Windows, prevent macros from colliding
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

// Sound class for managing individual sound effects
class Sound {
public:
    Sound(const std::string& filePath);
    virtual ~Sound();

    // Basic operations
    bool Play();
    void Stop();
    void Pause();
    void Resume();

    // Volume control (0.0 to 1.0)
    void SetVolume(float volume);
    float GetVolume() const;

    // Playback control
    void SetPitch(float pitch);
    float GetPitch() const;
    void SetPan(float pan); // -1.0 (left) to 1.0 (right)
    float GetPan() const;

    // Looping
    void SetLooping(bool loop);
    bool IsLooping() const;

    // Positional audio for 2D games
    void SetPosition(float x, float y, float z = 0.0f);
    void SetVelocity(float x, float y, float z = 0.0f);

    // Set the min/max distance for attenuation
    void SetAttenuationRange(float minDistance, float maxDistance);

    // Status checks
    bool IsPlaying();
    bool IsPaused() const;
    bool IsLoaded() const;

    // Get the duration of the sound in seconds
    float GetDuration() const;

    // Get the current playback position in seconds
    float GetPlaybackPosition() const;

    // Set the playback position in seconds
    void SetPlaybackPosition(float positionInSeconds);

    // Set category for volume management
    void SetCategory(AudioCategory category);
    AudioCategory GetCategory() const;

    // Called when the audio engine changes volumes
    void UpdateVolume();

    // Set a callback to be called when the sound finishes playing
    void SetFinishedCallback(std::function<void()> callback);

private:
    friend class AudioEngine;

    ma_sound sound;
    ma_engine* engine;
    std::string filePath;
    bool loaded;
    float volume;
    AudioCategory category;
    std::function<void()> finishedCallback;

    // Internal state tracking
    bool playing;
    bool paused;
};