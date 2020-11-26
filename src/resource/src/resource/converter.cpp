#include "converter.h"
#include "core\helper\debug.h"
#include "core\helper\timer.h"
#include "core\serialization\jsonArchive.h"

namespace Cjing3D
{
	ResConverterContext::ResConverterContext(BaseFileSystem& filesystem) :
		mFileSystem(filesystem)
	{
		mSerializer = CJING_NEW(JsonArchive)(ArchiveMode::ArchiveMode_Write, filesystem);
	}

	ResConverterContext::~ResConverterContext()
	{
		CJING_SAFE_DELETE(mSerializer);
	}

	void ResConverterContext::AddDependency(const char* filePath)
	{
	}

	bool ResConverterContext::Convert(IResConverter* converter, const ResourceType& resType, const char* srcPath, const char* destPath)
	{
		mMetaFilePath = srcPath;
		mMetaFilePath.append(".metadata");

		// create directory
		MaxPathString dirPath;
		if (!Path(destPath).SplitPath(dirPath.data(), dirPath.size()))
		{
			Debug::Warning("Invalid path:%s", destPath);
			return false;
		}
		mFileSystem.CreateDir(dirPath.c_str());

		// converting
		auto time = Timer::GetAbsoluteTime();
		Logger::Info("[Resource] Converting \"%s\"", srcPath);
		bool ret = converter->Convert(*this, resType, srcPath, destPath);
		Logger::Info("[Resource] Convert finished \"%s\" in %.2f ms.", srcPath, Timer::GetAbsoluteTime() - time);;

		// write metadata file
		if (ret == true)
		{
			mSerializer->Write("srcFile",  String(srcPath));
			mSerializer->Write("destFile", String(destPath));
			mSerializer->Save(mMetaFilePath.c_str());
		}
		return ret;
	}
}