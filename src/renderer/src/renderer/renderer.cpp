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

	void PresentBegin(GPU::CommandList& cmd)
	{
		GPU::PresentBegin(cmd);
	}

	void PresentEnd()
	{
		GPU::PresentEnd();
	}

	void EndFrame()
	{
		GPU::EndFrame();
	}
}
}