#include "buildConfig.h"
#include "json\json.hpp"
#include "core\filesystem\filesystem_physfs.h"
#include "core\helper\debug.h"

namespace Cjing3D
{
namespace BuildConfig
{
	namespace {
		nlohmann::json mRootJson;
		std::string mBuildCmd; //String mBuildCmd;
		std::string mProfile;  //String mProfile;
	}

	void Initialize(BaseFileSystem* filesystem)
	{
		const char* configName = "build_config.json";
		if (!filesystem->IsFileExists(configName)) {
			Logger::Warning("Failed to open build config \"build_config.json\"");
		}

		char* buffer = nullptr;
		U32 size = 0;
		if (!filesystem->ReadFile(configName, &buffer, size)) {
			CJING_SAFE_FREE(buffer);
		}

		try
		{
			mRootJson = nlohmann::json::parse(buffer, buffer + size);
		}
		catch (const std::exception& e)
		{
			Logger::Warning("Failed to open build config \"build_config.json\"");
		}

		// build cmd
		nlohmann::json::iterator it = mRootJson.find("build");
		if (it != mRootJson.end()) 
		{
			mBuildCmd = it->get<std::string>();
			Logger::Info("[BuildConfig] build cmd: %s", mBuildCmd.c_str());
		}
		// profile
		it = mRootJson.find("profile");
		if (it != mRootJson.end())
		{
			mProfile = it->get<std::string>();
			Logger::Info("[BuildConfig] profile: %s", mProfile.c_str());
		}

		CJING_SAFE_FREE(buffer);
	}

	String GetBuildCmd()
	{
		return mBuildCmd;
	}

	String GetProfile()
	{
		return mProfile;
	}
}
}