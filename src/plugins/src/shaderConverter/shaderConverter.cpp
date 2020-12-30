#include "shaderConverter.h"
#include "shaderParser.h"
#include "core\memory\linearAllocator.h"
#include "core\serialization\jsonArchive.h"
#include "core\helper\debug.h"
#include "core\string\stringUtils.h"

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

				// create input data
				mInputOffset = 0;
				mInputSize = StringLength(source) + 1;
				mInputData = (char*)mAllocator.Allocate(mInputSize);
				Memory::Memset(mInputData, 0, mInputSize);
				CopyString(Span(mInputData, mInputSize), source);

				// do preprocess
				Concurrency::ScopedMutex lock(mMutex);
				return fppPreProcess(mTags.data()) == 0;
			}

			static void FppError(void* userData, char* format, va_list varArgs)
			{
				StaticString<128> errMsg;
				sprintf(errMsg.data(), format, varArgs);
				Debug::Warning(errMsg);
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

	void ShaderMetaObject::Serialize(JsonArchive& archive)
	{

	}

	void ShaderMetaObject::Unserialize(JsonArchive& archive) const
	{

	}

	bool ShaderResConverter::Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest)
	{
		ShaderMetaObject data = context.GetMetaData<ShaderMetaObject>();

		BaseFileSystem& fileSystem = context.GetFileSystem();
		DynamicArray<char> source;
		if (!fileSystem.ReadFile(src, source)) {
			return false;
		}
		context.AddSource(src);

		// 1. preprocess source, acquire all dependencies
		MaxPathString rootPath;
		if (!Path(src).SplitPath(rootPath.data(), rootPath.size()))
		{
			Debug::Warning("[ShaderConverter] Invalid path:%s", src);
			return false;
		}

		ShaderPreprocessor preprocessor;
		preprocessor.AddInclude(rootPath.c_str());
		if (!preprocessor.Preprocess(src, source.data()))
		{
			Debug::Warning("[ShaderConverter] failed to preprocess shader source.");
			return false;
		}

		// record dependencies
		for (const auto& dep : preprocessor.GetDependencies()) {
			context.AddSource(dep);
		}

		// 2. parse shader source
		ShaderParser parser;
		auto shaderFileNode = parser.Parse(src, preprocessor.GetOutput());
		if (!shaderFileNode)
		{
			Debug::Warning("[ShaderConverter] failed to parse shader source.");
			return false;
		}

		// 3. compile shader source
#ifdef CJING3D_RENDERER_DX11
		ShaderCompilerHLSL shaderCompiler;
#endif


		// 4. write compiled file

		return false;

		/*if (!fileSystem.WriteFile(dest, buffer.data(), buffer.size())) {
			return false;
		}
		else
		{
			context.AddOutput(dest);
			context.SetMetaData<ShaderMetaObject>(data);
			return true;
		}*/
	}

	bool ShaderResConverter::SupportsType(const char* ext, const ResourceType& type)
	{
		return type == ResourceType("Shader") && EqualString(ext, "esf");
	}

	LUMIX_PLUGIN_ENTRY(shaderConverter)
	{
		ResConverterPlugin* plugin = CJING_NEW(ResConverterPlugin);
		plugin->CreateConverter = []() -> IResConverter* {
			return CJING_NEW(ShaderResConverter);
		};
		plugin->DestroyConverter = [](IResConverter*& converter) {
			CJING_SAFE_DELETE(converter);
			converter = nullptr;
		};
		return plugin;
	}
}