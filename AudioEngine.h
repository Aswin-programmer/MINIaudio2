#pragma once

#include "miniaudio.h"   

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>

class Sound;
class Music;

enum class AudioCategory {
    SFX,
    MUSIC,
    VOICE,
    AMBIENT
};

class AudioEngine {
public:
    static AudioEngine& Instance() {
        static AudioEngine instance;
        return instance;
    }

    bool Initialize();
    void Shutdown();

    // Sound management
    std::shared_ptr<Sound> LoadSound(const std::string& filePath);
    std::shared_ptr<Music> LoadMusic(const std::string& filePath);

    // Global volume control
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;

    // Category volume control
    void SetCategoryVolume(AudioCategory category, float volume);
    float GetCategoryVolume(AudioCategory category) const;

    // Mute/unmute
    void MuteAll(bool mute);
    void MuteCategory(AudioCategory category, bool mute);
    bool IsCategoryMuted(AudioCategory category) const;

    // Pause/resume all audio
    void PauseAll();
    void ResumeAll();

    // Stop all sounds
    void StopAll();

    // Device enumeration
    std::vector<std::string> GetAudioDevices() const;
    bool SetAudioDevice(const std::string& deviceName);
    std::string GetCurrentDevice() const;

    // Internal use (called by Sound/Music classes)
    ma_engine* GetEngine() { return &engine; }
    void RegisterSound(Sound* sound);
    void UnregisterSound(Sound* sound);
    void RegisterMusic(Music* music);
    void UnregisterMusic(Music* music);

    // 3D Audio settings
    void SetListenerPosition(float x, float y, float z = 0.0f);
    void SetListenerDirection(float x, float y, float z = 0.0f);
    void SetListenerVelocity(float x, float y, float z = 0.0f);
    void SetListenerWorldUp(float x, float y, float z = 1.0f);

    // Update method to be called once per frame
    void Update(float deltaTime);

private:
    AudioEngine();
    ~AudioEngine();

    // Prevent copying
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    ma_engine engine;
    ma_device_info* pPlaybackDeviceInfos;
    ma_uint32 playbackDeviceCount;
    ma_context context;

    float masterVolume;
    std::unordered_map<AudioCategory, float> categoryVolumes;
    std::unordered_map<AudioCategory, bool> categoryMuted;

    std::vector<Sound*> activeSounds;
    std::vector<Music*> activeMusic;

    std::string currentDevice;
    bool initialized;

    std::mutex soundMutex;
    std::mutex musicMutex;
};