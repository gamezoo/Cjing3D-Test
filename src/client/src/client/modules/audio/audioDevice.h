#pragma once

#include "core\common\common.h"

namespace Cjing3D
{
namespace Audio
{
	enum AUDIO_SUBMIX_TYPE
	{
		AUDIO_SUBMIX_TYPE_SOUNDEFFECT,
		AUDIO_SUBMIX_TYPE_MUSIC,
		AUDIO_SUBMIX_TYPE_USER0,
		AUDIO_SUBMIX_TYPE_USER1,
		AUDIO_SUBMIX_TYPE_COUNT,
	};

	struct SoundResource
	{
		SharedPtr<void> mInst = nullptr;
		inline bool IsValid()const { return mInst.get() != nullptr; }
		inline void Clear() { mInst = nullptr; }
	};

	struct SoundInstance
	{
		AUDIO_SUBMIX_TYPE mType = AUDIO_SUBMIX_TYPE_SOUNDEFFECT;
		float mLoopBegin = 0;	// (0 = from beginning)
		float mLoopLength = 0;	// (0 = until the end)

		SharedPtr<void> mInst = nullptr;
		inline bool IsValid()const { return mInst.get() != nullptr; }
		inline void Clear() { mInst = nullptr; }
	};

	struct SoundInstance3D
	{
		F32x3 listenerPos      = F32x3(0.0f, 0.0f, 0.0f);
		F32x3 listenerUp       = F32x3(0.0f, 1.0f, 0.0f);
		F32x3 listenerFront    = F32x3(0.0f, 0.0f, 1.0f);
		F32x3 listenerVelocity = F32x3(0.0f, 0.0f, 0.0f);
		F32x3 emitterPos	   = F32x3(0.0f, 0.0f, 0.0f);
		F32x3 emitterUp        = F32x3(0.0f, 1.0f, 0.0f);
		F32x3 emitterFront     = F32x3(0.0f, 0.0f, 1.0f);
		F32x3 emitterVelocity  = F32x3(0.0f, 0.0f, 0.0f);
		F32   emitterRadius    = 0.0f;
	};

	class AudioDevice
	{
	public:
		AudioDevice() {};
		virtual ~AudioDevice() {};

		static AudioDevice* Create();
		static void Destroy(AudioDevice* device);

		virtual bool Initialize() = 0;
		virtual void Uninitialize() = 0;
		virtual void Update(F32 delatTime) = 0;
		virtual bool LoadSoundImpl(const char* data, size_t length, SoundResource& resource) = 0;

		virtual bool CreateInstance(const SoundResource& resource, SoundInstance& inst) = 0;
		virtual void Play(SoundInstance& inst) = 0;
		virtual void Pause(SoundInstance& inst) = 0;
		virtual void Stop(SoundInstance& inst) = 0;
		virtual void SetVolume(SoundInstance& inst, F32 volume) = 0;
		virtual void SetMasteringVolume(F32 volume) = 0;

		virtual void Update3D(SoundInstance& instance, const SoundInstance3D& instance3D) = 0;
	};
}
}