#pragma once

#include "resource.h"
#include "core\plugin\plugin.h"
#include "core\container\dynamicArray.h"
#include "core\filesystem\filesystem.h"
#include "core\serialization\serializedObject.h"

namespace Cjing3D
{
	class IResConverter;
	class JsonArchive;

	class ResConverterContext
	{
	public:
		ResConverterContext(BaseFileSystem& filesystem);
		virtual ~ResConverterContext();

		void Load(const char* srcPath);
		void Clear();
		void AddSource(const char* path);
		void AddOutput(const char* path);
		bool Convert(IResConverter* converter, const ResourceType& resType, const char* srcPath, const char* destPath);
		bool WriteResource(const char* path, const U8* data, U32 size);

		template<typename T>
		void SetMetaData(const T& data)
		{
			SetMetaDataImpl(data);
		}
		template<typename T>
		T GetMetaData()
		{
			T data;
			GetMetaDataImpl(data);
			return data;
		}

		BaseFileSystem& GetFileSystem() { return mFileSystem; }
		const Path& GetMetaFilePath()const { return mMetaPath; }
		const Path& GetSrcPath()const { return mSrcPath; }

	private:
		void SetMetaDataImpl(const SerializedObject& obj);
		void GetMetaDataImpl(SerializedObject& obj);

	private:
		BaseFileSystem& mFileSystem;
		Path mMetaPath;
		Path mSrcPath;

		DynamicArray<String> mDependencies;
		DynamicArray<String> mSources;
		DynamicArray<String> mOutputs;
	};

	class IResConverter
	{
	public:
		virtual ~IResConverter() {}

		virtual void OnEditorGUI(ResConverterContext& context, const ResourceType& type, Resource* res) = 0;
		virtual bool SupportsType(const char* ext, const ResourceType& type) = 0;
		virtual bool Convert(ResConverterContext& context, const ResourceType& type, const char* src, const char* dest) = 0;
	};

	class ResConverterPlugin : public Plugin
	{
	public:
		PULGIN_DECLARE(ResConverter)

		// create specific converter
		typedef IResConverter* (*CreateConverterFunc)();
		CreateConverterFunc CreateConverter = nullptr;

		// destroy specific converter
		typedef void (*DestroyConverterFunc)(IResConverter*&);
		DestroyConverterFunc DestroyConverter = nullptr;
	};
}