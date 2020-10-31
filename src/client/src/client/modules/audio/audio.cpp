#include "audio.h"

namespace Cjing3D
{
	AudioManager::AudioManager()
	{
	}

	AudioManager::~AudioManager()
	{
	}

	void AudioManager::Initialize()
	{
		mAudioDevice = Audio::AudioDevice::Create();
	}

	void AudioManager::Uninitialize()
	{
		if (mAudioDevice != nullptr)
		{
			Audio::AudioDevice::Destroy(mAudioDevice);
			mAudioDevice = nullptr;
		}
	}

	void AudioManager::Update(F32 delta)
	{
		if (mAudioDevice != nullptr){
			mAudioDevice->Update(delta);
		}
	}
}