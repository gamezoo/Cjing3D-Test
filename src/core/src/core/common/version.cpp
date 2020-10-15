#include "version.h"
#include "core\string\string.h"

namespace CjingVersion
{
	// major version
	const int MajorVersion = 0;
	// features [0-100]
	const int MinorVersion = 2;
	// patch update; bug fixes [0-1000]
	const int PatchVersion = 0;
	// version string
	const Cjing3D::String VersionString = std::to_string(MajorVersion) + "." + std::to_string(MinorVersion) + "." + std::to_string(PatchVersion);

	int GetVersion()
	{
		return MajorVersion * 100000 + MinorVersion * 1000 + PatchVersion;
	}

	const char* GetVersionString()
	{
		return VersionString.c_str();
	}
}


