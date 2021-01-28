#pragma once

#include "gpu\definitions.h"
#include "shaderAST.h"
#include "core\container\set.h"
#include "core\container\staticArray.h"
#include "core\container\dynamicArray.h"

namespace Cjing3D
{
#define SHADER_STRUCT_INTERNAL "internal"

	using ShaderMap = StaticArray<Set<String>, GPU::SHADERSTAGES::SHADERSTAGES_COUNT>;

	struct ShaderBinding
	{
		ShaderBinding(I32 slot, const String& name) : mSlot(slot), mName(name) {}

		I32 mSlot;
		String mName;
	};

	struct ShaderCompileOutput
	{
		const U8* mByteCode = nullptr;
		U32 mByteCodeSize = 0;
		String mErrMsg;
		String mEntryPoint;
		GPU::SHADERSTAGES mStage;

		// binding
		DynamicArray<ShaderBinding> mCbuffers;
		DynamicArray<ShaderBinding> mSRVs;
		DynamicArray<ShaderBinding> mUAVs;
		DynamicArray<ShaderBinding> mSamplers;

		explicit operator bool()const { return mByteCodeSize > 0; }
	};

	class ShaderCompiler
	{
	public:
		ShaderCompiler(const char* srcPath) {}
		virtual ~ShaderCompiler() {}

		virtual bool GenerateAndCompile(ShaderAST::FileNode* fileNode, DynamicArray<String>& techFunctions, ShaderMap& shaderMap, DynamicArray<ShaderCompileOutput>& outputs) = 0;
		virtual ShaderCompileOutput Compile(const char* code, const char* entryPoint, GPU::SHADERSTAGES stage, I32 major, I32 minor) = 0;
	};
}
