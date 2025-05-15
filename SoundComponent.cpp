#include "SoundComponent.h"
#include "AudioEngine.h"
#include <algorithm>
#include <random>

std::unordered_map<std::string, std::shared_ptr<Sound>> SoundComponent::sounds;
std::unordered_map<std::string, std::shared_ptr<Music>> SoundComponent::music;

SoundComponent::SoundComponent()
    : posX(0.0f)
    , posY(0.0f)
    , posZ(0.0f)
    , velX(0.0f)
    , velY(0.0f)
    , velZ(0.0f)
    , currentSequenceIndex(0)
    , sequenceTimer(0.0f)
    , playingSequence(false)
{
}

SoundComponent::~SoundComponent() {
    StopAllSounds();
    StopAllMusic();
}

void SoundComponent::AddSound(const std::string& name, const std::string& filePath, AudioCategory category) {
    // Load the sound through the audio engine
    std::shared_ptr<Sound> sound = AudioEngine::Instance().LoadSound(filePath);
    if (sound) {
        sound->SetCategory(category);
        sounds[name] = sound;
    }
}

void SoundComponent::AddMusic(const std::string& name, const std::string& filePath, AudioCategory category) {
    // Load the music through the audio engine
    std::shared_ptr<Music> musicTrack = AudioEngine::Instance().LoadMusic(filePath);
    if (musicTrack) {
        musicTrack->SetCategory(category);
        music[name] = musicTrack;
    }
}

bool SoundComponent::PlaySound(const std::string& name, bool loop) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        // Apply randomization if set
        ApplySoundRandomization(name, it->second);

        // Set position for spatial audio
        it->second->SetPosition(posX, posY, posZ);
        it->second->SetVelocity(velX, velY, velZ);

        // Set looping
        it->second->SetLooping(loop);

        // Play the sound
        return it->second->Play();
    }
    return false;
}

bool SoundComponent::PlayMusic(const std::string& name, bool loop) {
    auto it = music.find(name);
    if (it != music.end()) {
        it->second->SetLooping(loop);
        return it->second->Play();
    }
    return false;
}

bool SoundComponent::PlayMusicWithFadeIn(const std::string& name, float fadeInDuration, bool loop) {
    auto it = music.find(name);
    if (it != music.end()) {
        it->second->SetLooping(loop);
        it->second->FadeIn(fadeInDuration);
        return true;
    }
    return false;
}

void SoundComponent::StopMusicWithFadeOut(const std::string& name, float fadeOutDuration) {
    auto it = music.find(name);
    if (it != music.end()) {
        it->second->FadeOut(fadeOutDuration, true);
    }
}

void SoundComponent::CrossFadeMusic(const std::string& oldMusic, const std::string& newMusic, float fadeDuration) {
    // Fade out the old music
    StopMusicWithFadeOut(oldMusic, fadeDuration);

    // Fade in the new music
    PlayMusicWithFadeIn(newMusic, fadeDuration, true);
}

void SoundComponent::StopSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        it->second->Stop();
    }
}

void SoundComponent::StopMusic(const std::string& name) {
    auto it = music.find(name);
    if (it != music.end()) {
        it->second->Stop();
    }
}

void SoundComponent::StopAllSounds() {
    for (auto& pair : sounds) {
        pair.second->Stop();
    }
}

void SoundComponent::StopAllMusic() {
    for (auto& pair : music) {
        pair.second->Stop();
    }
}

void SoundComponent::PauseSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        it->second->Pause();
    }
}

void SoundComponent::ResumeSound(const std::string& name) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        it->second->Resume();
    }
}

void SoundComponent::PauseMusic(const std::string& name) {
    auto it = music.find(name);
    if (it != music.end()) {
        it->second->Pause();
    }
}

void SoundComponent::ResumeMusic(const std::string& name) {
    auto it = music.find(name);
    if (it != music.end()) {
        it->second->Resume();
    }
}

void SoundComponent::SetSoundVolume(const std::string& name, float volume) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        it->second->SetVolume(volume);
    }
}

void SoundComponent::SetMusicVolume(const std::string& name, float volume) {
    auto it = music.find(name);
    if (it != music.end()) {
        it->second->SetVolume(volume);
    }
}

void SoundComponent::SetPosition(float x, float y, float z) {
    posX = x;
    posY = y;
    posZ = z;

    // Update position for all playing sounds
    for (auto& pair : sounds) {
        if (pair.second->IsPlaying()) {
            pair.second->SetPosition(x, y, z);
        }
    }
}

void SoundComponent::SetAttenuationRange(float min, float max)
{
    AttenuationRangeMin = min;
    AttenuationRangeMax = max;

    // Update position for all playing sounds
    for (auto& pair : sounds) {
        if (pair.second->IsPlaying()) {
            pair.second->SetAttenuationRange(min, max);
        }
    }
}

void SoundComponent::SetVelocity(float x, float y, float z) {
    velX = x;
    velY = y;
    velZ = z;

    // Update velocity for all playing sounds
    for (auto& pair : sounds) {
        if (pair.second->IsPlaying()) {
            pair.second->SetVelocity(x, y, z);
        }
    }
}

void SoundComponent::AddSoundTrigger(const std::string& soundName, SoundTriggerType triggerType,
    float parameter, const std::string& eventName) {
    // Make sure the sound exists
    if (sounds.find(soundName) == sounds.end()) {
        return;
    }

    SoundTrigger trigger;
    trigger.type = triggerType;
    trigger.parameter = parameter;
    trigger.eventName = eventName;
    trigger.accumulator = 0.0f;
    trigger.active = true;

    soundTriggers[soundName] = trigger;
}

void SoundComponent::RemoveSoundTrigger(const std::string& soundName, SoundTriggerType triggerType) {
    auto it = soundTriggers.find(soundName);
    if (it != soundTriggers.end() && it->second.type == triggerType) {
        soundTriggers.erase(it);
    }
}

void SoundComponent::TriggerEvent(const std::string& eventName) {
    // Check for any sounds that should be triggered by this event
    for (auto& pair : soundTriggers) {
        if (pair.second.type == SoundTriggerType::ON_EVENT &&
            pair.second.eventName == eventName &&
            pair.second.active) {

            PlaySound(pair.first, false);
        }
    }
}

void SoundComponent::Update(float deltaTime, float distanceToListener) {
    // Update music tracks (for fading)
    for (auto& pair : music) {
        pair.second->Update(deltaTime);
    }

    // Process sound triggers
    for (auto& pair : soundTriggers) {
        const std::string& soundName = pair.first;
        SoundTrigger& trigger = pair.second;

        switch (trigger.type) {
        case SoundTriggerType::ON_TIMER:
            trigger.accumulator += deltaTime;
            if (trigger.accumulator >= trigger.parameter) {
                PlaySound(soundName, false);
                trigger.accumulator = 0.0f;
            }
            break;

        case SoundTriggerType::ON_DISTANCE:
            // Only process if distance is provided
            if (distanceToListener >= 0.0f && trigger.active) {
                if (distanceToListener <= trigger.parameter) {
                    PlaySound(soundName, false);
                    trigger.active = false; // Prevent repeated triggers
                }
            }
            // Reset when moving away
            else if (distanceToListener > trigger.parameter * 1.5f) {
                trigger.active = true;
            }
            break;

        default:
            // Other trigger types are handled elsewhere
            break;
        }
    }

    // Update sound sequence if active
    if (playingSequence && !sequenceItems.empty()) {
        sequenceTimer += deltaTime;

        if (sequenceTimer >= sequenceItems[currentSequenceIndex].delay) {
            // Play the sound
            PlaySound(sequenceItems[currentSequenceIndex].soundName, false);

            // Move to next sound in sequence
            currentSequenceIndex++;
            sequenceTimer = 0.0f;

            // End of sequence?
            if (currentSequenceIndex >= sequenceItems.size()) {
                playingSequence = false;
            }
        }
    }
}

bool SoundComponent::IsSoundPlaying(const std::string& name) const {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        return it->second->IsPlaying();
    }
    return false;
}

bool SoundComponent::IsMusicPlaying(const std::string& name) const {
    auto it = music.find(name);
    if (it != music.end()) {
        return it->second->IsPlaying();
    }
    return false;
}

void SoundComponent::SetRandomPitchRange(const std::string& soundName, float minPitch, float maxPitch) {
    RandomRange range;
    range.enabled = true;
    range.min = std::max(0.5f, minPitch);
    range.max = std::min(2.0f, maxPitch);

    pitchRanges[soundName] = range;
}

void SoundComponent::SetRandomVolumeRange(const std::string& soundName, float minVolume, float maxVolume) {
    RandomRange range;
    range.enabled = true;
    range.min = std::max(0.0f, minVolume);
    range.max = std::min(1.0f, maxVolume);

    volumeRanges[soundName] = range;
}

void SoundComponent::PlaySoundSequence(const std::vector<std::string>& soundNames, const std::vector<float>& delays) {
    // Validate inputs
    if (soundNames.empty() || soundNames.size() != delays.size()) {
        return;
    }

    // Stop any existing sequence
    StopSoundSequence();

    // Setup the new sequence
    sequenceItems.clear();
    for (size_t i = 0; i < soundNames.size(); i++) {
        SoundSequenceItem item;
        item.soundName = soundNames[i];
        item.delay = delays[i];
        sequenceItems.push_back(item);
    }

    // Start the sequence
    currentSequenceIndex = 0;
    sequenceTimer = 0.0f;
    playingSequence = true;
}

void SoundComponent::StopSoundSequence() {
    if (playingSequence && currentSequenceIndex < sequenceItems.size()) {
        StopSound(sequenceItems[currentSequenceIndex].soundName);
    }

    playingSequence = false;
    sequenceItems.clear();
}

void SoundComponent::ApplySoundRandomization(const std::string& name, std::shared_ptr<Sound> sound) {
    // Apply random pitch if configured
    auto pitchIt = pitchRanges.find(name);
    if (pitchIt != pitchRanges.end() && pitchIt->second.enabled) {
        sound->SetPitch(GetRandomFloat(pitchIt->second.min, pitchIt->second.max));
    }

    // Apply random volume if configured
    auto volumeIt = volumeRanges.find(name);
    if (volumeIt != volumeRanges.end() && volumeIt->second.enabled) {
        sound->SetVolume(GetRandomFloat(volumeIt->second.min, volumeIt->second.max));
    }
}

float SoundComponent::GetRandomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}