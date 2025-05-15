#include "miniaudio.h"
#include "Music.h"
#include "AudioEngine.h"

Music::Music(const std::string& filePath)
    : filePath(filePath)
    , loaded(false)
    , volume(1.0f)
    , category(AudioCategory::MUSIC)
    , playing(false)
    , paused(false)
    , fadeState(FadeState::None)
    , fadeDuration(0.0f)
    , fadeTimer(0.0f)
    , fadeStartVolume(0.0f)
    , fadeTargetVolume(0.0f)
    , stopAfterFadeOut(false)
{
    engine = AudioEngine::Instance().GetEngine();

    // Initialize the music - use MA_SOUND_FLAG_STREAM for efficient streaming
    ma_result result = ma_sound_init_from_file(engine, filePath.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &sound);
    loaded = (result == MA_SUCCESS);

    if (loaded) {
        // Register with the audio engine
        AudioEngine::Instance().RegisterMusic(this);
    }
}

Music::~Music() {
    if (loaded) {
        Stop(); // Ensure the music is stopped
        ma_sound_uninit(&sound);
        AudioEngine::Instance().UnregisterMusic(this);
    }
}

bool Music::Play() {
    if (!loaded) return false;

    ma_result result = ma_sound_start(&sound);
    if (result == MA_SUCCESS) {
        playing = true;
        paused = false;
        return true;
    }
    return false;
}

void Music::Stop() {
    if (!loaded) return;

    ma_sound_stop(&sound);
    ma_sound_seek_to_pcm_frame(&sound, 0); // Reset position
    playing = false;
    paused = false;
    fadeState = FadeState::None;
}

void Music::Pause() {
    if (!loaded || !playing || paused) return;

    ma_sound_stop(&sound);
    paused = true;
}

void Music::Resume() {
    if (!loaded || !paused) return;

    ma_sound_start(&sound);
    paused = false;
}

void Music::SetVolume(float vol) {
    volume = std::max(0.0f, std::min(vol, 1.0f));
    UpdateVolume();
}

float Music::GetVolume() const {
    return volume;
}

void Music::SetPitch(float pitch) {
    if (!loaded) return;

    // Clamp pitch to reasonable values
    pitch = std::max(0.5f, std::min(pitch, 2.0f));
    ma_sound_set_pitch(&sound, pitch);
}

float Music::GetPitch() const {
    if (!loaded) return 1.0f;

    return ma_sound_get_pitch(&sound);
}

void Music::SetPan(float pan) {
    if (!loaded) return;

    // Clamp pan between -1.0 (left) and 1.0 (right)
    pan = std::max(-1.0f, std::min(pan, 1.0f));
    ma_sound_set_pan(&sound, pan);
}

float Music::GetPan() const {
    if (!loaded) return 0.0f;

    return ma_sound_get_pan(&sound);
}

void Music::SetLooping(bool loop) {
    if (!loaded) return;

    ma_sound_set_looping(&sound, loop);
}

bool Music::IsLooping() const {
    if (!loaded) return false;

    return ma_sound_is_looping(&sound);
}

void Music::FadeIn(float durationInSeconds) {
    if (!loaded || durationInSeconds <= 0.0f) {
        // Just play normally if duration is invalid
        Play();
        return;
    }

    // Setup fade in
    fadeState = FadeState::FadingIn;
    fadeDuration = durationInSeconds;
    fadeTimer = 0.0f;
    fadeStartVolume = 0.0f;
    fadeTargetVolume = volume;

    // Start with zero volume
    float effectiveVolume = 0.0f;
    ma_sound_set_volume(&sound, effectiveVolume);

    // Start playback
    Play();
}

void Music::FadeOut(float durationInSeconds, bool stopAfterFade) {
    if (!loaded || !playing || durationInSeconds <= 0.0f) {
        // Just stop immediately if duration is invalid or not playing
        if (stopAfterFade) {
            Stop();
        }
        return;
    }

    // Setup fade out
    fadeState = FadeState::FadingOut;
    fadeDuration = durationInSeconds;
    fadeTimer = 0.0f;
    fadeStartVolume = ma_sound_get_volume(&sound);
    fadeTargetVolume = 0.0f;
    stopAfterFadeOut = stopAfterFade;
}

bool Music::IsPlaying() {
    if (!loaded) return false;

    if (paused) return false;

    // Check with the miniaudio API
    ma_bool32 isPlaying = ma_sound_is_playing(&sound);

    // Update our internal state
    if (isPlaying == MA_FALSE && playing) {
        // Sound has finished playing
        playing = false;
        if (finishedCallback) {
            finishedCallback();
        }
    }

    return isPlaying == MA_TRUE;
}

bool Music::IsPaused() const {
    return paused;
}

bool Music::IsLoaded() const {
    return loaded;
}

bool Music::IsFading() const {
    return fadeState != FadeState::None;
}

float Music::GetDuration() const {
    if (!loaded) return 0.0f;

    float durationInSeconds = 0.0f;
    ma_sound_get_length_in_seconds(const_cast<ma_sound*>(&sound), &durationInSeconds);
    return durationInSeconds;
}

float Music::GetPlaybackPosition() const {
    if (!loaded) return 0.0f;

    float positionInSeconds = 0.0f;
    ma_sound_get_cursor_in_seconds(const_cast<ma_sound*>(&sound), &positionInSeconds);
    return positionInSeconds;
}

void Music::SetPlaybackPosition(float positionInSeconds) {
    if (!loaded) return;

    float duration = GetDuration();
    if (positionInSeconds < 0) positionInSeconds = 0;
    if (positionInSeconds > duration) positionInSeconds = duration;

    ma_uint64 frameCount = 0;
    ma_uint32 sampleRate = ma_engine_get_sample_rate(engine);
    frameCount = (ma_uint64)(positionInSeconds * sampleRate);

    ma_sound_seek_to_pcm_frame(&sound, frameCount);
}

void Music::SetCategory(AudioCategory cat) {
    category = cat;
    UpdateVolume();
}

AudioCategory Music::GetCategory() const {
    return category;
}

void Music::UpdateVolume() {
    if (!loaded) return;

    float effectiveVolume = volume;

    // Apply category volume
    effectiveVolume *= AudioEngine::Instance().GetCategoryVolume(category);

    // Apply mute if applicable
    if (AudioEngine::Instance().IsCategoryMuted(category)) {
        effectiveVolume = 0.0f;
    }

    // If we're in the middle of fading, don't disrupt the fade
    if (fadeState != FadeState::None) {
        // Store the target volume for fading
        fadeTargetVolume = effectiveVolume;
    }
    else {
        ma_sound_set_volume(&sound, effectiveVolume);
    }
}

void Music::SetFinishedCallback(std::function<void()> callback) {
    finishedCallback = callback;
}

void Music::Update(float deltaTime) {
    if (!loaded || fadeState == FadeState::None) return;

    // Process fading
    fadeTimer += deltaTime;
    float fadeProgress = fadeTimer / fadeDuration;

    if (fadeProgress >= 1.0f) {
        // Fade complete
        if (fadeState == FadeState::FadingOut && stopAfterFadeOut) {
            Stop();
        }
        else {
            ma_sound_set_volume(&sound, fadeTargetVolume);
            fadeState = FadeState::None;
        }
    }
    else {
        // Fade in progress
        float currentVolume;
        if (fadeState == FadeState::FadingIn) {
            currentVolume = fadeStartVolume + (fadeTargetVolume - fadeStartVolume) * fadeProgress;
        }
        else { // FadingOut
            currentVolume = fadeStartVolume + (fadeTargetVolume - fadeStartVolume) * fadeProgress;
        }
        ma_sound_set_volume(&sound, currentVolume);
    }
}