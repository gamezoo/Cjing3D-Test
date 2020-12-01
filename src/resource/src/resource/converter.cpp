#include "converter.h"
#include "core\helper\debug.h"
#include "core\helper\timer.h"
#include "core\serialization\jsonArchive.h"
#include "core\jobsystem\jobsystem.h"

namespace Cjing3D
{
	ResConverterContext::ResConverterContext(BaseFileSystem& filesystem) :
		mFileSystem(filesystem)
	{
		mDeserializer = CJING_NEW(JsonArchive)(ArchiveMode::ArchiveMode_Write, filesystem);
	}

	ResConverterContext::~ResConverterContext()
	{
		CJING_SAFE_DELETE(mDeserializer);
		CJING_SAFE_DELETE(mSerializer);
	}

	void ResConverterContext::AddDependency(const char* filePath)
	{
		mDependencies.push(filePath);
	}

	void ResConverterContext::AddSource(const char* path)
	{
		mSources.push(path);
	}

	void ResConverterContext::AddOutput(const char* path)
	{
		mOutputs.push(path);
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
			// try remove old metadata file
			while (mFileSystem.IsFileExists(mMetaFilePath))
			{
				mFileSystem.DeleteFile(mMetaFilePath);
				JobSystem::YieldCPU();
			}

			mDeserializer->Write("dependencies",  mDependencies);
			mDeserializer->Write("sources", mSources);
			mDeserializer->Write("outputs", mOutputs);
			mDeserializer->Save(mMetaFilePath.c_str());
		}
		return ret;
	}

	void ResConverterContext::SetMetaDataImpl(const SerializedObject& obj)
	{
		if (mDeserializer->GetMode() == ArchiveMode::ArchiveMode_Read)
		{
			CJING_SAFE_DELETE(mDeserializer);
			mDeserializer = CJING_NEW(JsonArchive)(ArchiveMode::ArchiveMode_Write, mFileSystem);
		}

		obj.Unserialize(*mDeserializer);
	}

	void ResConverterContext::GetMetaDataImpl(SerializedObject& obj)
	{
		if (mSerializer && mSerializer->GetMode() == ArchiveMode::ArchiveMode_Read) {
			obj.Serialize(*mSerializer);
		}

		if (!mFileSystem.IsFileExists(mMetaFilePath)) {
			return;
		}

		CJING_SAFE_DELETE(mSerializer);
		mSerializer = CJING_NEW(JsonArchive)(ArchiveMode::ArchiveMode_Read, mFileSystem);
		obj.Serialize(*mSerializer);
	}
}