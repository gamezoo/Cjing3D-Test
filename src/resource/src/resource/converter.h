#pragma once

#include "resource.h"
#include "core\plugin\plugin.h"
#include "core\container\dynamicArray.h"
#include "core\filesystem\filesystem.h"

namespace Cjing3D
{
	class IResConverter;
	class JsonArchive;

	class ResConverterContext
	{
	public:
		ResConverterContext(BaseFileSystem& filesystem);
		virtual ~ResConverterContext();

		void AddDependency(const char* filePath);
		bool Convert(IResConverter* converter, const ResourceType& resType, const char* srcPath, const char* destPath);
	
	private:
		DynamicArray<String> mDependencies;
		BaseFileSystem& mFileSystem;
		MaxPathString mMetaFilePath;
		JsonArchive* mSerializer;
	};

	class IResConverter
	{
	public:
		virtual ~IResConverter() {}

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