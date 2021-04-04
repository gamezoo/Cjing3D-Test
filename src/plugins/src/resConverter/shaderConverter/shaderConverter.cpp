#include "shaderConverter.h"
#include "shaderParser.h"
#include "shaderMetadata.h"
#include "shaderEditor.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"
#include "renderer\shaderImpl.h"

#include "gpu\definitions.h"

#ifdef CJING3D_RENDERER_DX11
#include "shaderCompilerHLSL.h"
#endif

extern "C" {
#include "fcpp\src\cppdef.h"
#include "fcpp\src\fpp.h"
}

namespace Cjing3D
{
	namespace
	{
		/// ///////////////////////////////////////////////////////////////////////
		/// ShaderPreprocessor
		class ShaderPreprocessor
		{
		private:
			LinearAllocator mAllocator;
			DynamicArray<fppTag> mTags;
			DynamicArray<const char*> mDependencies;
			char* mInputData = nullptr;
			I32 mInputSize = 0;
			I32 mInputOffset = 0;
			String mOutput;
			static Concurrency::Mutex mMutex;

		public:
			ShaderPreprocessor() 
			{
				mAllocator.Reserve(1024 * 1024);

				fppTag tag;
				tag.tag = FPPTAG_USERDATA;
				tag.data = this;
				mTags.push(tag);

				tag.tag = FPPTAG_INPUT;
				tag.data = (void*)FppInput;
				mTags.push(tag);

				tag.tag = FPPTAG_OUTPUT;
				tag.data = (void*)FppOutput;
				mTags.push(tag);

				tag.tag = FPPTAG_ERROR;
				tag.data = (void*)FppError;
				mTags.push(tag);

				tag.tag = FPPTAG_DEPENDENCY;
				tag.data = (void*)FppDependency;
				mTags.push(tag);

				tag.tag = FPPTAG_IGNOREVERSION;
				tag.data = (void*)0;
				mTags.push(tag);

				tag.tag = FPPTAG_LINE;
				tag.data = (void*)1;
				mTags.push(tag);

				tag.tag = FPPTAG_KEEPCOMMENTS;
				tag.data = (void*)0;
				mTags.push(tag);
			}

			~ShaderPreprocessor() {}

			void AddInclude(const char* path)
			{
				I32 size = StringLength(path) + 1;
				char* data = (char*)mAllocator.Allocate(size);
				CopyString(Span(data, size), path);

				fppTag tag;
				tag.tag = FPPTAG_INCLUDE_DIR;
				tag.data = data;
				mTags.push(tag);
			}

			bool Preprocess(const char* path, const char* source)
			{
				fppTag tag;
				tag.tag = FPPTAG_INPUT_NAME;
				tag.data = (void*)path;
				mTags.push(tag);

				// end of tag list
				tag.tag = FPPTAG_END;
				tag.data = (void*)0;
				mTags.push(tag);

				// convert to unix line endings
				String srcBuffer = StringUtils::ReplaceString(source, "\r\n", "\n");

				// create input data
				mInputOffset = 0;
				mInputSize = srcBuffer.length() + 1;
				mInputData = (char*)mAllocator.Allocate(mInputSize);
				Memory::Memset(mInputData, 0, mInputSize);
				CopyString(Span(mInputData, mInputSize), srcBuffer.c_str());

				// do preprocess
				Concurrency::ScopedMutex lock(mMutex);
				return fppPreProcess(mTags.data()) == 0;
			}

			static void FppError(void* userData, char* format, va_list varArgs)
			{
				StaticString<128> errMsg;
				Logger::Error(errMsg.Sprintfv(format, varArgs));
			}

			static char* FppInput(char* buffer, int size, void* userData)
			{
				auto* preprocessor = static_cast<ShaderPreprocessor*>(userData);
				I32 outIndex = 0;
				char c = preprocessor->mInputData[preprocessor->mInputOffset];
				while (preprocessor->mInputOffset < preprocessor->mInputSize && outIndex < (size - 1))
				{
					buffer[outIndex] = c;
					if (c == '\n' || outIndex == (size - 1))
					{
						buffer[++outIndex] = '\0';
						preprocessor->mInputOffset++;
						return buffer;
					}
					outIndex++;
					c = preprocessor->mInputData[++preprocessor->mInputOffset];
				}
				return nullptr;
			}

			static void FppOutput(int inChar, void* userData)
			{
				auto* preprocessor = static_cast<ShaderPreprocessor*>(userData);
				char str[] = { (char)inChar, '\0' };
				preprocessor->mOutput.append(str);
			}

			static void FppDependency(char* dependency, void* userData)
			{
				auto* preprocessor = static_cast<ShaderPreprocessor*>(userData);

				I32 size = StringLength(dependency) + 1;
				char* str = (char*)preprocessor->mAllocator.Allocate(size);
				CopyString(Span(str, size), dependency);

				preprocessor->mDependencies.push(str);
			}

			const DynamicArray<const char*>& GetDependencies()const { return mDependencies; }
			const char* GetOutput()const { return mOutput.c_str(); }
		};
		Concurrency::Mutex ShaderPreprocessor::mMutex;
	}

	void ShaderMetaObject::Serialize(JsonArchive& archive)const {}
	void ShaderMetaObject::Unserialize(JsonArchive& archive) {}

	bool ShaderResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		ShaderMetaObject data = context.GetMetaData<ShaderMetaObject>();

		BaseFileSystem& fileSystem = context.GetFileSystem();
		DynamicArray<char> source;
		if (!fileSystem.ReadFile(src, source)) {
			return false;
		}
		context.AddSource(src);

		//////////////////////////////////////////////////////////////////////////////////
		// 1. preprocess source, acquire all dependencies
		MaxPathString parentPath;
		if (!Path(src).SplitPath(parentPath.data(), parentPath.size()))
		{
			Logger::Warning("[ShaderConverter] Invalid path:%s", src);
			return false;
		}

		// get shader root path
		Path fullRootPath = context.GetFileSystem().GetBasePath();
		Path fullSrcPath = fullRootPath;

		fullRootPath.AppendPath(parentPath.c_str());	
		fullSrcPath.AppendPath(src);

		ShaderPreprocessor preprocessor;
		preprocessor.AddInclude(fullRootPath.c_str());
		preprocessor.AddInclude(SHADER_INTEROP_PATH);

		if (!preprocessor.Preprocess(fullSrcPath.c_str(), source.data()))
		{
			Logger::Warning("[ShaderConverter] failed to preprocess shader source.");
			return false;
		}

		// record dependencies
		for (const auto& dep : preprocessor.GetDependencies()) {
			context.AddSource(dep);
		}

		//////////////////////////////////////////////////////////////////////////////////
		// 2. parse shader source into an shader ast
		ShaderParser parser;
		auto shaderFileNode = parser.Parse(src, preprocessor.GetOutput());
		if (!shaderFileNode)
		{
			Logger::Warning("[ShaderConverter] failed to parse shader source.");
			return false;
		}

		//////////////////////////////////////////////////////////////////////////////////
		// 3. parse shader ast to create shader metadata
		ShaderAST::ShaderMetadata shaderMetadata;
		shaderFileNode->Visit(&shaderMetadata);
		
		// get tech and shaders from shader metadata
		DynamicArray<String> techFunctions;
		ShaderMap shaderMap;

		const auto& techniques = shaderMetadata.GetTechniques();
		for (const auto& tech : techniques)
		{
			if (!tech.mVS.empty()) 
			{
				techFunctions.push(tech.mVS);
				shaderMap[GPU::SHADERSTAGES::SHADERSTAGES_VS].insert(tech.mVS);
			}
			if (!tech.mHS.empty())
			{
				techFunctions.push(tech.mHS);
				shaderMap[GPU::SHADERSTAGES::SHADERSTAGES_HS].insert(tech.mHS);
			}
			if (!tech.mDS.empty())
			{
				techFunctions.push(tech.mDS);
				shaderMap[GPU::SHADERSTAGES::SHADERSTAGES_DS].insert(tech.mDS);
			}
			if (!tech.mPS.empty())
			{
				techFunctions.push(tech.mPS);
				shaderMap[GPU::SHADERSTAGES::SHADERSTAGES_PS].insert(tech.mPS);
			}
			if (!tech.mCS.empty()) 
			{
				techFunctions.push(tech.mCS);
				shaderMap[GPU::SHADERSTAGES::SHADERSTAGES_CS].insert(tech.mCS);
			}
			if (!tech.mGS.empty())
			{
				shaderMap[GPU::SHADERSTAGES::SHADERSTAGES_GS].insert(tech.mGS);
			}
		}

		//////////////////////////////////////////////////////////////////////////////////
		// 4. compile shader source
#ifdef CJING3D_RENDERER_DX11
		// ShaderModel version (dx11 only support sm5.0)
		I32 majorVer = 5;
		I32 minorVer = 0;
		ShaderCompilerHLSL shaderCompiler(src, fullRootPath.c_str(), context, majorVer, minorVer);
#endif
		DynamicArray<ShaderCompileOutput> compileOutput;
		if (!shaderCompiler.GenerateAndCompile(shaderFileNode, techFunctions, shaderMap, compileOutput))
		{
			Logger::Warning("[ShaderConverter] failed to generate and compile shader source.");
			return false;
		}	

		//////////////////////////////////////////////////////////////////////////////////
		// 5. process binding set
		// record binding infos
		auto AddBindings = [](const DynamicArray<ShaderBinding>& bindings, HashMap<String, I32>& outBindings) {
			for (const auto& binding : bindings)
			{
				if (outBindings.find(binding.mName) == nullptr) {
					outBindings.insert(binding.mName, binding.mSlot);
				}
			}
		};
		HashMap<String, I32> allBindings;
		for (const auto& compile : compileOutput)
		{
			AddBindings(compile.mCbuffers, allBindings);
			AddBindings(compile.mSRVs, allBindings);
			AddBindings(compile.mUAVs, allBindings);
			AddBindings(compile.mSamplers, allBindings);
		}
		
		// bindingSetHeaders
		DynamicArray<ShaderBindingHeader> bindingHeaders;
		auto PopulateBindings = [&bindingHeaders, &allBindings](const DynamicArray<String>& resources, ShaderBindingFlags flags) {
			bool isUsed = false;
			I32 index = 0;
			for (const auto& resource : resources)
			{
				ShaderBindingHeader bindingHeader;
				CopyString(bindingHeader.mName, resource.c_str());

				auto it = allBindings.find(resource);
				if (it != nullptr) 
				{
					isUsed = true;
					bindingHeader.mSlot = *it;
				}

				bindingHeader.mHandle = (I32)flags | (index++ & (I32)ShaderBindingFlags::INDEX_MASK);
				bindingHeaders.push(bindingHeader);
			}
			return isUsed;
		};
		auto& bindingSets = shaderMetadata.GetBindingSets();
		DynamicArray<ShaderBindingSetHeader> bindingSetHeaders;
		DynamicArray<I32> bindingSetHeaderIndexs;
		bindingSetHeaders.reserve(bindingSets.size());
		bindingSetHeaderIndexs.reserve(bindingSets.size());

		for (int i = 0; i < bindingSets.size(); i++)
		{
			const auto& bindingSet = bindingSets[i];
			// check is used
			bool isUsed = false;
			isUsed |= PopulateBindings(bindingSet.mCBVs, ShaderBindingFlags::CBV);
			isUsed |= PopulateBindings(bindingSet.mSRVs, ShaderBindingFlags::SRV);
			isUsed |= PopulateBindings(bindingSet.mUAVs, ShaderBindingFlags::UAV);
			isUsed |= PopulateBindings(bindingSet.mSamplers, ShaderBindingFlags::SAMPLER);
			if (isUsed)
			{
				ShaderBindingSetHeader bindingSetHeader;
				CopyString(bindingSetHeader.mName, bindingSet.mName);
				bindingSetHeader.mIsShared = bindingSet.mIsShared;
				bindingSetHeader.mNumCBVs = bindingSet.mCBVs.size();
				bindingSetHeader.mNumSRVs = bindingSet.mSRVs.size();
				bindingSetHeader.mNumUAVs = bindingSet.mUAVs.size();
				bindingSetHeader.mNumSamplers = bindingSet.mSamplers.size();

				bindingSetHeaders.push(bindingSetHeader);
				bindingSetHeaderIndexs.push(i);
			}
			else
			{
				I32 totalNumRes = bindingSet.mCBVs.size() + bindingSet.mSRVs.size() +
					bindingSet.mUAVs.size() + bindingSet.mSamplers.size();
				bindingHeaders.pop(totalNumRes);
			}
		}

		//////////////////////////////////////////////////////////////////////////////////
		// 6. Static samplerStates
		DynamicArray<ShaderSamplerStateHeader> samplerStateHeaders;
		const auto& samplerStates = shaderMetadata.GetSamplerStates();
		for (const auto& samplerState : samplerStates)
		{
			auto& header = samplerStateHeaders.emplace();
			CopyString(header.mName, samplerState.mName.c_str());
			header.mSamplerState = samplerState.mDesc;
			header.mSlot = samplerState.mSlot;
		}

		//////////////////////////////////////////////////////////////////////////////////
		// 7. prepare shader headers for writing
		DynamicArray<ShaderBytecodeHeader> bytecodeHeaders;
		I32 offset = 0;
		for (const auto& compileInfo : compileOutput)
		{
			ShaderBytecodeHeader& bytecodeHeader = bytecodeHeaders.emplace();
			bytecodeHeader.mStage = compileInfo.mStage;
			bytecodeHeader.mOffset = offset;
			bytecodeHeader.mBytes = compileInfo.mByteCodeSize;

			offset += compileInfo.mByteCodeSize;
		}

		// technique headers
		DynamicArray<ShaderTechniqueHeader> techniqueHeaders;
		auto FindShaderIdx = [&](const char* name) -> I32 {
			if (!name) {
				return -1;
			}
			I32 idx = 0;
			for (const auto& compile : compileOutput)
			{
				if (compile.mEntryPoint == name) {
					return idx;
				}
				idx++;
			}
			return -1;
		};
		const auto& renderStates = shaderMetadata.GetRenderStates();
		for (const auto& tech : techniques)
		{
			ShaderTechniqueHeader& header = techniqueHeaders.emplace();
			CopyString(header.mName, tech.mName.c_str());
			header.mIdxVS = FindShaderIdx(tech.mVS);
			header.mIdxGS = FindShaderIdx(tech.mGS);
			header.mIdxDS = FindShaderIdx(tech.mDS);
			header.mIdxHS = FindShaderIdx(tech.mHS);
			header.mIdxPS = FindShaderIdx(tech.mPS);
			header.mIdxCS = FindShaderIdx(tech.mCS);

			// find index of renderState
			I32 idxRenderState = -1;
			for (int i = 0; i < renderStates.size(); i++)
			{
				if (renderStates[i].mName == tech.mRenderState.mName)
				{
					idxRenderState = i;
					break;
				}
			}
			header.mIdxRenderState = idxRenderState;

			// process used bindingSets
			HashMap<String, I32> techBindings;
			auto AddTechBindings = [&](I32 shaderIndex)
			{
				if (shaderIndex != -1)
				{
					auto& compile = compileOutput[shaderIndex];
					AddBindings(compile.mCbuffers, techBindings);
					AddBindings(compile.mSRVs, techBindings);
					AddBindings(compile.mUAVs, techBindings);
					AddBindings(compile.mSamplers, techBindings);
				}
			};
			AddTechBindings(header.mIdxVS);
			AddTechBindings(header.mIdxGS);
			AddTechBindings(header.mIdxDS);
			AddTechBindings(header.mIdxHS);
			AddTechBindings(header.mIdxPS);
			AddTechBindings(header.mIdxCS);

			// record used bindingSet index
			auto CheckBindingSetUsed = [&techBindings](const DynamicArray<String>& resources)->bool {
				for (const auto& resource : resources)
				{
					if (techBindings.find(resource) != nullptr) {
						return true;
					}
				}
				return false;
			};
			I32 numUsedBindingSlot = 0;
			for (I32 index = 0; index < bindingSetHeaders.size(); index++)
			{
				const auto& bindingSet = bindingSets[bindingSetHeaderIndexs[index]];		
				bool isUsed = CheckBindingSetUsed(bindingSet.mCBVs);
				isUsed = isUsed | CheckBindingSetUsed(bindingSet.mSRVs);
				isUsed = isUsed | CheckBindingSetUsed(bindingSet.mUAVs);
				isUsed = isUsed | CheckBindingSetUsed(bindingSet.mSamplers);
				if (isUsed) {
					header.mBindingSetIndexs[numUsedBindingSlot++] = index;
				}
			}
			header.mNumBindingSet = numUsedBindingSlot;
		}

		// Tech hashers
		DynamicArray<ShaderTechHasherHeader> techHasherHeaders;
		auto FindTechIdx = [&](const char* name) -> I32 {
			if (!name) {
				return -1;
			}
			I32 idx = 0;
			for (const auto& tech : techniqueHeaders)
			{
				if (EqualString(tech.mName, name)) {
					return idx;
				}
				idx++;
			}
			return -1;
		};

		const auto& techHashers = shaderMetadata.GetTechHashers();
		for (const auto& techHasher : techHashers)
		{
			auto& hasher = techHasherHeaders.emplace();
			hasher.mIdxTech = FindTechIdx(techHasher.mTech);
			hasher.mHasher = techHasher.mHasher;
		}

		//////////////////////////////////////////////////////////////////////////////////
		// 8. write shader

		// Converted file format:
		// | GeneralHeader 
		// | BindingSetHeaders 
		// | BindingHeaders 
		// | SamplerStateHeaders
		// | BytecodeHeaders 
		// | TechniqueHeaders 
		// | RenderStateHeaders
		// | RenderStateJson
		// | TechHashers
		// | bytecodes

		ShaderSerializer serializer;
		File* file = CJING_NEW(File);
		if (!fileSystem.OpenFile(dest, *file, FileFlags::DEFAULT_WRITE)) 
		{
			CJING_SAFE_DELETE(file);
			return false;
		}

		// general header
		ShaderGeneralHeader generalHeader;
		generalHeader.mNumBindingSets = bindingSetHeaders.size();
		generalHeader.mNumSamplerStates = samplerStateHeaders.size();
		generalHeader.mNumShaders = compileOutput.size();
		generalHeader.mNumTechniques = techniques.size();
		generalHeader.mNumRenderStates = renderStates.size();
		generalHeader.mNumTechHashers = techHasherHeaders.size();

		file->Write(&generalHeader, sizeof(generalHeader));
		if (!bindingSetHeaders.empty()) {
			file->Write(bindingSetHeaders.data(), bindingSetHeaders.size() * sizeof(ShaderBindingSetHeader));
		}
		if (!bindingHeaders.empty()) {
			file->Write(bindingHeaders.data(), bindingHeaders.size() * sizeof(ShaderBindingHeader));
		}
		if (!samplerStateHeaders.empty()) {
			file->Write(samplerStateHeaders.data(), samplerStateHeaders.size() * sizeof(ShaderSamplerStateHeader));
		}
		if (!bytecodeHeaders.empty()) {
			file->Write(bytecodeHeaders.data(), bytecodeHeaders.size() * sizeof(ShaderBytecodeHeader));
		}
		if (!techniqueHeaders.empty()) {
			file->Write(techniqueHeaders.data(), techniqueHeaders.size() * sizeof(ShaderTechniqueHeader));
		}
		if (!renderStates.empty())
		{
			// write renderStates as json
			DynamicArray<RenderStateHeader> renderStateHeaders;
			I32 offset = 0;

			String mOutputRenderStates;
			for (const auto& renderState : renderStates)
			{
				JsonArchive archive(ArchiveMode::ArchiveMode_Write);
				serializer.SerializeRenderState(renderState.mDesc, archive);

				const auto& output = archive.DumpJsonString();
				if (output.empty()) {
					continue;
				}
				mOutputRenderStates.append(output);

				auto& renderStateHeader = renderStateHeaders.emplace();
				renderStateHeader.mOffset = offset;
				renderStateHeader.mBytes = mOutputRenderStates.size() - offset;
				offset = mOutputRenderStates.size();
			}

			file->Write(renderStateHeaders.data(), renderStateHeaders.size() * sizeof(RenderStateHeader));
			file->Write(mOutputRenderStates.data(), mOutputRenderStates.size());
		}
		if (!techHasherHeaders.empty()) {
			file->Write(techHasherHeaders.data(), techHasherHeaders.size() * sizeof(ShaderTechHasherHeader));
		}

		// write bytecode
		for (const auto& compileInfo : compileOutput) {
			file->Write(compileInfo.mByteCode, compileInfo.mByteCodeSize);
		}

		context.AddOutput(dest);
		context.SetMetaData<ShaderMetaObject>(data);

		CJING_SAFE_DELETE(file);
		return true;
	}

	void ShaderResConverter::OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res)
	{
		static ShaderEditor editor;
		editor.OnEditorGUI(context, type, res);
	}

	bool ShaderResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Shader") && EqualString(ext, "jsf");
	}
}