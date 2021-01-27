#pragma once

#include "gpu\definitions.h"
#include "gpu\resource.h"
#include "renderer\shader.h"
#include "core\serialization\jsonArchive.h"

namespace Cjing3D
{
	static const I32 SHADER_MAX_NAME_LENGTH = 64;
	static const I32 SHADER_MAX_BOUND_BINDING_SETS = 16;

	struct ShaderTechniqueImpl;

	struct ShaderGeneralHeader
	{
		static const U32 MAGIC;
		static const I32 MAJOR = 0;
		static const I32 MINOR = 1;

		U32 mMagic = MAGIC;
		I32 mMajor = MAJOR;
		I32 mMinor = MINOR;

		I32 mNumBindingSets = 0;
		I32 mNumShaders = 0;
		I32 mNumTechniques = 0;
		I32 mNumRenderStates = 0;
	};

	struct RenderStateHeader
	{
		I32 mOffset = 0;
		I32 mBytes = 0;
	};

	enum class ShaderBindingFlags :U32
	{
		INVALID = 0,

		CBV = 0x1000,
		SRV = 0x2000,
		UAV = 0x3000,

		TYPE_MASK = 0xf000,
		INDEX_MASK = 0x0fff,
	};

	struct ShaderBindingHeader
	{
		char mName[SHADER_MAX_NAME_LENGTH] = { '\0' };
		I32 mHandle = 0;
		I32 mSlot = 0;
	};

	struct ShaderBindingSetHeader
	{
		char mName[SHADER_MAX_NAME_LENGTH] = { '\0' };
		bool mIsShared = false;
		I32 mNumCBVs = 0;
		I32 mNumSRVs = 0;
		I32 mNumUAVs = 0;
	};

	struct ShaderBytecodeHeader
	{
		GPU::SHADERSTAGES mStage = GPU::SHADERSTAGES_COUNT;
		I32 mOffset = 0;
		I32 mBytes = 0;
	};

	struct ShaderTechniqueHeader
	{
		char mName[SHADER_MAX_NAME_LENGTH] = {'\0'};
		I32 mIdxVS = -1;
		I32 mIdxGS = -1;
		I32 mIdxHS = -1;
		I32 mIdxDS = -1;
		I32 mIdxPS = -1;
		I32 mIdxCS = -1;
		I32 mIdxRenderState = -1;
		I32 mNumBindingSet = 0;
		I32 mBindingSetIndexs[SHADER_MAX_BOUND_BINDING_SETS];
	};

	struct ShaderImpl
	{
	public:
		String mName;
		ShaderGeneralHeader mGeneralHeader;
		DynamicArray<ShaderBindingSetHeader> mBindingSetHeaders;
		DynamicArray<ShaderBindingHeader> mBindingHeaders;
		DynamicArray<ShaderBytecodeHeader> mBytecodeHeaders;
		DynamicArray<ShaderTechniqueHeader> mTechniqueHeaders;
		DynamicArray<RenderStateHeader> mRenderStateHeaders;
		DynamicArray<char> mBytecodes;
		DynamicArray<GPU::ResHandle> mRhiShaders;
		Concurrency::RWLock mRWLoclk;

		// renderStates
		DynamicArray<GPU::RenderStateDesc> mRenderStates;

		// technique
		DynamicArray<U64> mTechniquesHashes;
		DynamicArray<ShaderTechniqueImpl*> mTechniques;
		DynamicArray<ShaderTechniqueDesc> mTechniqueDescs;
		DynamicArray<GPU::ResHandle> mPipelineStates;

		bool SetupTechnique(ShaderTechniqueImpl* technique);

	public:
		ShaderImpl();
		~ShaderImpl();

		ShaderTechniqueImpl* CreateTechnique(const char* name, const ShaderTechniqueDesc& desc);
	};

	struct ShaderTechniqueImpl
	{
	public:
		String mName;
		ShaderTechniqueHeader mHeader;
		ShaderImpl* mShaderImpl = nullptr;
		I32 mIndex = -1;

		GPU::ResHandle GetPipelineState()const;
		bool IsValid()const;
	};

	struct ShaderBindingSetImpl
	{
		ShaderBindingSetHeader mHeader;
		I32 mIndex = 0;
		GPU::ResHandle mBindingSetHandle;

		DynamicArray<GPU::BindingBuffer> mCBVs;
		DynamicArray<GPU::BindingSRV> mSRVs;
		DynamicArray<GPU::BindingUAV> mUAVs;

		bool Initialize();
	};

	struct ShaderSerializer
	{
		// renderTargetBlend
		void SerializeRenderTargetBlend(const GPU::RenderTargetBlendStateDesc& desc, JsonArchive& archive);
		void UnserializeRenderTargetBlend(GPU::RenderTargetBlendStateDesc& desc, JsonArchive& archive);
		// depthStencilOp
		void SerializeDepthStencilOp(const GPU::DepthStencilOpDesc& desc, JsonArchive& archive);
		void UnserializeDepthStencilOpe(GPU::DepthStencilOpDesc& desc, JsonArchive& archive);
		// RenderState
		void SerializeRenderState(const GPU::RenderStateDesc& desc, JsonArchive& archive);
		void UnserializeRenderState(GPU::RenderStateDesc& desc, JsonArchive& archive);
	};
}