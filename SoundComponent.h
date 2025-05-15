#pragma once

#include "Sound.h"
#include "Music.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

enum class SoundTriggerType {
    ON_COLLISION,
    ON_DISTANCE,
    ON_EVENT,
    ON_TIMER
};

// Sound component for game entities
class SoundComponent {
public:
    SoundComponent();
    ~SoundComponent();

    // Load sounds and music
    static void AddSound(const std::string& name, const std::string& filePath, AudioCategory category = AudioCategory::SFX);
    static void AddMusic(const std::string& name, const std::string& filePath, AudioCategory category = AudioCategory::MUSIC);

    // Play sounds
    bool PlaySound(const std::string& name, bool loop = false);
    bool PlayMusic(const std::string& name, bool loop = true);

    // Special music playback
    bool PlayMusicWithFadeIn(const std::string& name, float fadeInDuration, bool loop = true);
    void StopMusicWithFadeOut(const std::string& name, float fadeOutDuration);
    void CrossFadeMusic(const std::string& oldMusic, const std::string& newMusic, float fadeDuration);

    // Stop specific sound/music
    void StopSound(const std::string& name);
    void StopMusic(const std::string& name);

    // Stop all sounds/music associated with this component
    void StopAllSounds();
    void StopAllMusic();

    // Pause/resume specific sound/music
    void PauseSound(const std::string& name);
    void ResumeSound(const std::string& name);
    void PauseMusic(const std::string& name);
    void ResumeMusic(const std::string& name);

    // Volume control for specific sounds/music
    void SetSoundVolume(const std::string& name, float volume);
    void SetMusicVolume(const std::string& name, float volume);

    // Entity position for spatial audio
    void SetPosition(float x, float y, float z = 0.0f);
    void SetVelocity(float x, float y, float z = 0.0f);

    // Entity Set AttenuationRange for spatial audio
    void SetAttenuationRange(float min, float max);

    // Sound triggering system
    void AddSoundTrigger(const std::string& soundName, SoundTriggerType triggerType,
        float parameter, const std::string& eventName = "");
    void RemoveSoundTrigger(const std::string& soundName, SoundTriggerType triggerType);

    // Event system
    void TriggerEvent(const std::string& eventName);

    // Update method to be called once per frame
    void Update(float deltaTime, float distanceToListener = -1.0f);

    // Check if a sound is currently playing
    bool IsSoundPlaying(const std::string& name) const;
    bool IsMusicPlaying(const std::string& name) const;

    // Randomization features
    void SetRandomPitchRange(const std::string& soundName, float minPitch, float maxPitch);
    void SetRandomVolumeRange(const std::string& soundName, float minVolume, float maxVolume);

    // Sound sequence - play a sequence of sounds with specified delays
    void PlaySoundSequence(const std::vector<std::string>& soundNames, const std::vector<float>& delays);
    void StopSoundSequence();

private:
    struct SoundTrigger {
        SoundTriggerType type;
        float parameter;  // distance, time, etc.
        std::string eventName;
        float accumulator; // For timer-based triggers
        bool active;
    };

    struct SoundSequenceItem {
        std::string soundName;
        float delay;
    };

    // Sound storage
    static std::unordered_map<std::string, std::shared_ptr<Sound>> sounds;
    static std::unordered_map<std::string, std::shared_ptr<Music>> music;

    // Position and velocity
    float posX, posY, posZ;
    float velX, velY, velZ;

    // AttenuationRange
    float AttenuationRangeMin, AttenuationRangeMax;

    // Sound triggers
    std::unordered_map<std::string, SoundTrigger> soundTriggers;

    // Random range storage
    struct RandomRange {
        bool enabled;
        float min;
        float max;
    };

    std::unordered_map<std::string, RandomRange> pitchRanges;
    std::unordered_map<std::string, RandomRange> volumeRanges;

    // Sound sequence state
    std::vector<SoundSequenceItem> sequenceItems;
    size_t currentSequenceIndex;
    float sequenceTimer;
    bool playingSequence;

    // Helper function to apply randomization
    void ApplySoundRandomization(const std::string& name, std::shared_ptr<Sound> sound);

    // Helper to get random float
    float GetRandomFloat(float min, float max);
};