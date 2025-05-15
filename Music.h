#pragma once

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

// Music class for streaming background music
class Music {
public:
    Music(const std::string& filePath);
    virtual ~Music();

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

    // Looping control
    void SetLooping(bool loop);
    bool IsLooping() const;

    // Fading
    void FadeIn(float durationInSeconds);
    void FadeOut(float durationInSeconds, bool stopAfterFade = true);

    // Status checks
    bool IsPlaying();
    bool IsPaused() const;
    bool IsLoaded() const;
    bool IsFading() const;

    // Get the duration of the music in seconds
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

    // Set a callback to be called when the music finishes playing
    void SetFinishedCallback(std::function<void()> callback);

    // Update method to be called once per frame (for fading)
    void Update(float deltaTime);

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

    // Fading state
    enum class FadeState {
        None,
        FadingIn,
        FadingOut
    };

    FadeState fadeState;
    float fadeDuration;
    float fadeTimer;
    float fadeStartVolume;
    float fadeTargetVolume;
    bool stopAfterFadeOut;
};