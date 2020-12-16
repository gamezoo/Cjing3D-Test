#include "renderer.h"

namespace Cjing3D
{
namespace Renderer
{
	//////////////////////////////////////////////////////////////////////////
	// Member
	//////////////////////////////////////////////////////////////////////////
	bool mIsInitialize = false;

	//////////////////////////////////////////////////////////////////////////
	// Member
	//////////////////////////////////////////////////////////////////////////
	void Initialize(GPU::GPUSetupParams params)
	{
		if (mIsInitialize) {
			return;
		}

		// initialize gpu 
		GPU::Initialize(params);

		mIsInitialize = true;
	}

	bool IsInitialized()
	{
		return mIsInitialize;
	}

	void Uninitialize()
	{
		if (!mIsInitialize) {
			return;
		}

		// uninitialize gpu
		GPU::Uninitialize();

		mIsInitialize = false;
	}

	void PresentBegin()
	{
		GPU::PresentBegin(GPU::ResHandle::INVALID_HANDLE);
	}

	void PresentEnd()
	{
		GPU::PresentEnd(GPU::ResHandle::INVALID_HANDLE);
	}

	void EndFrame()
	{
		GPU::EndFrame();
	}
}
}