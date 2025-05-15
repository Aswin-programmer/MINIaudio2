#include <iostream>

#include "AudioEngine.h"
#include "SoundComponent.h"

int main()
{
    // -----------------------
// 1) At startup (once):
// -----------------------
    if (!AudioEngine::Instance().Initialize()) {
        std::cerr << "Failed to initialize audio!\n";
        // handle error...
    }

    // Set up listener (usually follows player/camera)
    AudioEngine::Instance().SetListenerPosition(0.0f, 0.0f, 0.0f);  // X, Y, Z
    AudioEngine::Instance().SetListenerDirection(0.0f, 0.0f, -1.0f); // Facing forward
    AudioEngine::Instance().SetListenerWorldUp(0.0f, 1.0f, 0.0f);    // Y-up world

    //AudioEngine::Instance().SetMasterVolume(0.1);

    // Optionally pick a non-default device:
    // auto devices = AudioEngine::Instance().GetAudioDevices();
    // AudioEngine::Instance().SetAudioDevice(devices[1]);


    // -----------------------
    // 2) Load your assets:
    // -----------------------
    SoundComponent worldAudio;

    // Configure 3D properties
    worldAudio.SetPosition(1, 1, 0.0f);  // 25 units to the right
    worldAudio.SetAttenuationRange(1.0f, 20.0f); // Min 1m, Max 20m
    worldAudio.SetVelocity(0.0f, 0.0f, 0.0f);   // Stationary sound

    // a) load a sound effect
    worldAudio.AddSound("footstep", "ASSETS/SOUND/magic-spell.wav", AudioCategory::SFX);

    //// b) load background music
    //SoundComponent::AddMusic("bgm", "ASSETS/SOUND/magic-spell.wav", AudioCategory::MUSIC);

    // -----------------------
    // 3) Playing sounds & music:
    // -----------------------

    // play a one-shot effect
    worldAudio.PlaySound("footstep");

    //// start looping background music
    //if (!worldAudio.PlayMusic("bgm", true)) {
    //    std::cerr << "PlayMusic() returned false — could not start playback!\n";
    //}



    //// fade music in over 2 seconds
    //worldAudio.PlayMusicWithFadeIn("bgm", /*fadeInDuration=*/2.0f);

    //// later, fade it out
    //worldAudio.StopMusicWithFadeOut("bgm", /*fadeOutDuration=*/3.0f);


    // -----------------------
    // 4) In your game loop:
    // -----------------------
    while (true) {

        // Update the audio engine (cleans up finished sounds)
        AudioEngine::Instance().Update(0.016);

        // Update per-entity component (fade processing, triggers, sequences...)
        // If you have just one global component, pass distanceToListener = 0
        worldAudio.Update(0.016, /*distanceToListener=*/ -1.0f);

        // … rest of your game update & render …
    }


    AudioEngine::Instance().Shutdown();
}

