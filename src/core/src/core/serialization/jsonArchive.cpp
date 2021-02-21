#include "jsonArchive.h"
#include "core\helper\debug.h"
#include "core\filesystem\filesystem.h"

namespace Cjing3D
{
	const U32 JsonArchive::currentArchiveVersion = 1;

	JsonArchive::JsonArchive(ArchiveMode mode, BaseFileSystem* fileSystem) :
		ArchiveBase("", mode, fileSystem)
	{
	}

	JsonArchive::JsonArchive(ArchiveMode mode, const char* jsonStr, size_t size) :
		ArchiveBase("", mode, nullptr)
	{
		if (mMode == ArchiveMode::ArchiveMode_Read) 
		{
			try
			{
				mRootJson = nlohmann::json::parse(jsonStr, jsonStr + size);
			}
			catch (const std::exception& e)
			{
				Logger::Warning("Failed to parse json string: %s", e.what());
				Close();
			}
		}
	}

	JsonArchive::JsonArchive(const String& path, ArchiveMode mode, BaseFileSystem* fileSystem) :
		ArchiveBase(path, mode, fileSystem)
	{
		if (mMode == ArchiveMode::ArchiveMode_Read) {
			OpenJson(path.c_str());
		}
	}

	JsonArchive::~JsonArchive()
	{
		while (!mJsonStack.empty()) {
			mJsonStack.pop();
		}

		if (mMode == ArchiveMode::ArchiveMode_Write) {
			Save(mFilePath);
		}
	}

	void JsonArchive::OpenJson(const char* path)
	{
		if (!Load(path)) {
			return;
		}
		try
		{
			mRootJson = nlohmann::json::parse(mDataBuffer, mDataBuffer + mDataSize);
		}
		catch (const std::exception& e)
		{
			Logger::Warning("Fail to open json file:%s, %s", path, e.what());
			Close();
		}
	}

	void JsonArchive::SetPath(const char* path)
	{
		if (mMode != ArchiveMode::ArchiveMode_Write)
		{
			if (!mFilePath.empty() && !mRootJson.empty())
			{
				while (!mJsonStack.empty()) {
					mJsonStack.pop();
				}
				Close();
			}
			OpenJson(path);
		}
		ArchiveBase::SetPath(path);
	}

	bool JsonArchive::Save(const String& path)
	{
		if (!mFileSystem) {
			return false;
		}

		if (path.empty()) {
			return false;
		}

		String jsonString = mRootJson.dump(0);
		if (jsonString.empty()) {
			return false;
		}

		return mFileSystem->WriteFile(path, jsonString.c_str(), jsonString.size());
	}

	String JsonArchive::DumpJsonString() const
	{
		return mRootJson.dump(0);
	}

	nlohmann::json* JsonArchive::GetCurrentJson()
	{
		if (mJsonStack.empty()) {
			return &mRootJson;
		}
		return mJsonStack.top();
	}

	const nlohmann::json* JsonArchive::GetCurrentJson() const
	{
		if (mJsonStack.empty()) {
			return &mRootJson;
		}
		return mJsonStack.top();
	}

	size_t JsonArchive::GetCurrentValueCount() const
	{
		return GetCurrentJson()->size();
	}
}