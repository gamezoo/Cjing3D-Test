#include "converter.h"
#include "core\helper\debug.h"
#include "core\helper\timer.h"
#include "core\serialization\jsonArchive.h"
#include "core\concurrency\jobsystem.h"

namespace Cjing3D
{
	ResConverterContext::ResConverterContext(BaseFileSystem& filesystem) :
		mFileSystem(filesystem)
	{
	}

	ResConverterContext::~ResConverterContext()
	{
	}

	void ResConverterContext::Load(const char* srcPath)
	{
		// load complete metadata(sources, outputs), the orginal data will clear
		MaxPathString metaPath(srcPath);
		metaPath.append(".metadata");
		if (mMetaPath.c_str() == metaPath) {
			return;
		}

		Clear();

		if (!mFileSystem.IsFileExists(metaPath.c_str()))
		{
			Logger::Warning("The metadata of target path '%s'is dose not exist.");
			return;
		}
		mSrcPath = srcPath;
		mMetaPath = metaPath;

		JsonArchive archive(metaPath.c_str(), ArchiveMode::ArchiveMode_Read, &mFileSystem);
		// Unserialize internal info
		archive.Read("$internal", [this](JsonArchive& archive) {
			archive.Read("sources", mSources);
			archive.Read("outputs", mOutputs);
		});
	}

	void ResConverterContext::Clear()
	{
		mDependencies.clear();
		mSources.clear();
		mOutputs.clear();
		mMetaPath.Clear();
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
		mSrcPath = srcPath;
		mMetaPath = srcPath;
		mMetaPath.AppendString(".metadata");

		// create directory
		MaxPathString dirPath;
		if (!Path(destPath).SplitPath(dirPath.data(), dirPath.size()))
		{
			Logger::Warning("Invalid path:%s", destPath);
			return false;
		}
		mFileSystem.CreateDir(dirPath.c_str());

		// converting
		auto time = Timer::GetAbsoluteTime();
		Logger::Info("[Resource] Converting \"%s\"", srcPath);
		bool ret = converter->Convert(*this, resType, srcPath, destPath);
		Logger::Info("[Resource] Convert finished \"%s\" in %.2f ms.", srcPath, Timer::GetAbsoluteTime() - time);

		return ret;
	}

	void ResConverterContext::SetMetaDataImpl(const SerializedObject& obj)
	{
		if (mMetaPath.IsEmpty())
		{
			Logger::Warning("[ResConverterContext] Failed to set metadata and the target path is empty.");
			return;
		}

		// try remove old metadata file
		while (mFileSystem.IsFileExists(mMetaPath.c_str()))
		{
			mFileSystem.DeleteFile(mMetaPath.c_str());
			JobSystem::YieldCPU();
		}

		JsonArchive archive(mMetaPath.c_str(), ArchiveMode::ArchiveMode_Write, &mFileSystem);
		// serialize meta info
		obj.Serialize(archive);

		// serialize internal info
		archive.WriteCallback("$internal", [this](JsonArchive& archive) {
			archive.Write("sources", mSources);
			archive.Write("outputs", mOutputs);
		});
	}

	void ResConverterContext::GetMetaDataImpl(SerializedObject& obj)
	{
		if (!mFileSystem.IsFileExists(mMetaPath.c_str())) {
			return;
		}

		JsonArchive archive(mMetaPath.c_str(), ArchiveMode::ArchiveMode_Read, &mFileSystem);
		obj.Unserialize(archive);
	}
}