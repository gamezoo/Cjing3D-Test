#pragma once

#include "core\common\common.h"
#include "client\modules\module.h"
#include "audioDevice.h"

namespace Cjing3D
{
	class AudioManager : public Module
	{
	public:
		AudioManager();
		~AudioManager();

		void Initialize()override;
		void Uninitialize()override;
		void Update(F32 delta)override;

	private:
		Audio::AudioDevice* mAudioDevice = nullptr;
	};
}