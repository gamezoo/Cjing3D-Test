#include "jsonArchive.h"
#include "core\helper\debug.h"
#include "core\filesystem\filesystem.h"

namespace Cjing3D
{
	const U32 JsonArchive::currentArchiveVersion = 1;

	JsonArchive::JsonArchive(const String& path, ArchiveMode mode) :
		ArchiveBase(path, mode)
	{
		if (mMode == ArchiveMode::ArchiveMode_Read)
		{
			if (!Load(mFilePath)) {
				return;
			}
			try
			{
				mRootJson = nlohmann::json::parse(mDataBuffer, mDataBuffer + mDataSize);
			}
			catch (const std::exception& e)
			{
				Debug::Warning("Fail to open json file:%s, %s", mFilePath, e.what());
				Close();
				return;
			}
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

	bool JsonArchive::Save(const String& path)
	{
		String jsonString = mRootJson.dump(0);
		if (jsonString.empty()) {
			return false;
		}

		return FileSystem::SaveFile(path, jsonString.c_str(), jsonString.size());
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