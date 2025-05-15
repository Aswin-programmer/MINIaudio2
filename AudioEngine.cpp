#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "AudioEngine.h"
#include "Sound.h"
#include "Music.h"

AudioEngine::AudioEngine()
    : pPlaybackDeviceInfos(nullptr)
    , playbackDeviceCount(0)
    , masterVolume(1.0f)
    , initialized(false)
{
    // Initialize default category volumes
    categoryVolumes[AudioCategory::SFX] = 1.0f;
    categoryVolumes[AudioCategory::MUSIC] = 1.0f;
    categoryVolumes[AudioCategory::VOICE] = 1.0f;
    categoryVolumes[AudioCategory::AMBIENT] = 1.0f;

    // Initialize mute states
    categoryMuted[AudioCategory::SFX] = false;
    categoryMuted[AudioCategory::MUSIC] = false;
    categoryMuted[AudioCategory::VOICE] = false;
    categoryMuted[AudioCategory::AMBIENT] = false;
}

AudioEngine::~AudioEngine() {
    Shutdown();
}

bool AudioEngine::Initialize() {
    if (initialized) {
        return true;
    }

    // Initialize miniaudio context for device enumeration
    ma_result result = ma_context_init(nullptr, 0, nullptr, &context);
    if (result != MA_SUCCESS) {
        return false;
    }

    // Enumerate playback devices
    result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, nullptr, nullptr);
    if (result != MA_SUCCESS) {
        ma_context_uninit(&context);
        return false;
    }

    // Set the default device
    if (playbackDeviceCount > 0) {
        currentDevice = pPlaybackDeviceInfos[0].name;
    }

    // Initialize the engine
    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.listenerCount = 1;

    result = ma_engine_init(&engineConfig, &engine);
    if (result != MA_SUCCESS) {
        ma_context_uninit(&context);
        return false;
    }

    initialized = true;
    return true;
}

void AudioEngine::Shutdown() {
    if (!initialized) {
        return;
    }

    // Stop all sounds
    StopAll();

    // Uninitialize the engine
    ma_engine_uninit(&engine);
    ma_context_uninit(&context);

    initialized = false;
}

std::shared_ptr<Sound> AudioEngine::LoadSound(const std::string& filePath) {
    if (!initialized) {
        return nullptr;
    }

    std::shared_ptr<Sound> sound = std::make_shared<Sound>(filePath);
    if (sound->IsLoaded()) {
        return sound;
    }

    return nullptr;
}

std::shared_ptr<Music> AudioEngine::LoadMusic(const std::string& filePath) {
    if (!initialized) {
        return nullptr;
    }

    std::shared_ptr<Music> music = std::make_shared<Music>(filePath);
    if (music->IsLoaded()) {
        return music;
    }

    return nullptr;
}

void AudioEngine::SetMasterVolume(float volume) {
    masterVolume = std::max(0.0f, std::min(volume, 1.0f));
    ma_engine_set_volume(&engine, masterVolume);
}

float AudioEngine::GetMasterVolume() const {
    return masterVolume;
}

void AudioEngine::SetCategoryVolume(AudioCategory category, float volume) {
    categoryVolumes[category] = std::max(0.0f, std::min(volume, 1.0f));

    // Apply volume to all sounds in this category
    std::lock_guard<std::mutex> lock(soundMutex);
    for (auto sound : activeSounds) {
        if (sound->GetCategory() == category) {
            sound->UpdateVolume();
        }
    }

    std::lock_guard<std::mutex> lockMusic(musicMutex);
    for (auto music : activeMusic) {
        if (music->GetCategory() == category) {
            music->UpdateVolume();
        }
    }
}

float AudioEngine::GetCategoryVolume(AudioCategory category) const {
    auto it = categoryVolumes.find(category);
    if (it != categoryVolumes.end()) {
        return it->second;
    }
    return 1.0f;
}

void AudioEngine::MuteAll(bool mute) {
    for (auto& pair : categoryMuted) {
        pair.second = mute;
    }

    // Update all sounds and music
    std::lock_guard<std::mutex> lock(soundMutex);
    for (auto sound : activeSounds) {
        sound->UpdateVolume();
    }

    std::lock_guard<std::mutex> lockMusic(musicMutex);
    for (auto music : activeMusic) {
        music->UpdateVolume();
    }
}

void AudioEngine::MuteCategory(AudioCategory category, bool mute) {
    categoryMuted[category] = mute;

    // Apply mute to all sounds in this category
    std::lock_guard<std::mutex> lock(soundMutex);
    for (auto sound : activeSounds) {
        if (sound->GetCategory() == category) {
            sound->UpdateVolume();
        }
    }

    std::lock_guard<std::mutex> lockMusic(musicMutex);
    for (auto music : activeMusic) {
        if (music->GetCategory() == category) {
            music->UpdateVolume();
        }
    }
}

bool AudioEngine::IsCategoryMuted(AudioCategory category) const {
    auto it = categoryMuted.find(category);
    if (it != categoryMuted.end()) {
        return it->second;
    }
    return false;
}

void AudioEngine::PauseAll() {
    std::lock_guard<std::mutex> lock(soundMutex);
    for (auto sound : activeSounds) {
        sound->Pause();
    }

    std::lock_guard<std::mutex> lockMusic(musicMutex);
    for (auto music : activeMusic) {
        music->Pause();
    }
}

void AudioEngine::ResumeAll() {
    std::lock_guard<std::mutex> lock(soundMutex);
    for (auto sound : activeSounds) {
        sound->Resume();
    }

    std::lock_guard<std::mutex> lockMusic(musicMutex);
    for (auto music : activeMusic) {
        music->Resume();
    }
}

void AudioEngine::StopAll() {
    std::lock_guard<std::mutex> lock(soundMutex);
    for (auto sound : activeSounds) {
        sound->Stop();
    }

    std::lock_guard<std::mutex> lockMusic(musicMutex);
    for (auto music : activeMusic) {
        music->Stop();
    }
}

std::vector<std::string> AudioEngine::GetAudioDevices() const {
    std::vector<std::string> devices;

    for (ma_uint32 i = 0; i < playbackDeviceCount; i++) {
        devices.push_back(pPlaybackDeviceInfos[i].name);
    }

    return devices;
}

bool AudioEngine::SetAudioDevice(const std::string& deviceName) {
    if (!initialized) {
        return false;
    }

    // Find the device
    for (ma_uint32 i = 0; i < playbackDeviceCount; i++) {
        if (deviceName == pPlaybackDeviceInfos[i].name) {
            // Stop all current sounds
            StopAll();

            // Uninitialize the engine
            ma_engine_uninit(&engine);

            // Create a new engine with the selected device
            ma_engine_config engineConfig = ma_engine_config_init();
            engineConfig.pPlaybackDeviceID = &pPlaybackDeviceInfos[i].id;

            ma_result result = ma_engine_init(&engineConfig, &engine);
            if (result != MA_SUCCESS) {
                // Try to reinitialize with the default device
                engineConfig.pPlaybackDeviceID = nullptr;
                result = ma_engine_init(&engineConfig, &engine);
                if (result != MA_SUCCESS) {
                    return false;
                }
            }

            // Set the master volume
            ma_engine_set_volume(&engine, masterVolume);

            currentDevice = deviceName;
            return true;
        }
    }

    return false;
}

std::string AudioEngine::GetCurrentDevice() const {
    return currentDevice;
}

void AudioEngine::RegisterSound(Sound* sound) {
    if (!sound) return;

    std::lock_guard<std::mutex> lock(soundMutex);
    activeSounds.push_back(sound);
}

void AudioEngine::UnregisterSound(Sound* sound) {
    if (!sound) return;

    std::lock_guard<std::mutex> lock(soundMutex);
    auto it = std::find(activeSounds.begin(), activeSounds.end(), sound);
    if (it != activeSounds.end()) {
        activeSounds.erase(it);
    }
}

void AudioEngine::RegisterMusic(Music* music) {
    if (!music) return;

    std::lock_guard<std::mutex> lock(musicMutex);
    activeMusic.push_back(music);
}

void AudioEngine::UnregisterMusic(Music* music) {
    if (!music) return;

    std::lock_guard<std::mutex> lock(musicMutex);
    auto it = std::find(activeMusic.begin(), activeMusic.end(), music);
    if (it != activeMusic.end()) {
        activeMusic.erase(it);
    }
}

void AudioEngine::SetListenerPosition(float x, float y, float z) {
    ma_engine_listener_set_position(&engine, 0, x, y, z);
}

void AudioEngine::SetListenerDirection(float x, float y, float z) {
    ma_engine_listener_set_direction(&engine, 0, x, y, z);
}

void AudioEngine::SetListenerVelocity(float x, float y, float z) {
    ma_engine_listener_set_velocity(&engine, 0, x, y, z);
}

void AudioEngine::SetListenerWorldUp(float x, float y, float z) {
    ma_engine_listener_set_world_up(&engine, 0, x, y, z);
}

void AudioEngine::Update(float deltaTime) {
    // Clean up any finished sounds
    std::lock_guard<std::mutex> lock(soundMutex);
    auto soundIt = activeSounds.begin();
    while (soundIt != activeSounds.end()) {
        if (!(*soundIt)->IsPlaying()) {
            soundIt = activeSounds.erase(soundIt);
        }
        else {
            ++soundIt;
        }
    }
}