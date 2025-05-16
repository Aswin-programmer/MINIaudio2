#include "SoundSystem.h"

SoundSystem::SoundSystem()
{
    Initialize();
}

SoundSystem::~SoundSystem()
{
    AudioEngine::Instance().Shutdown();
}

void SoundSystem::Initialize()
{
    if (!AudioEngine::Instance().Initialize()) {
        std::cerr << "Failed to initialize audio!\n";
        // handle error...
    }
}

void SoundSystem::Update()
{
    AudioEngine::Instance().Update(0.016);
}
