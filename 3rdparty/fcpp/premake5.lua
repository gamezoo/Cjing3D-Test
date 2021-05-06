dofile("../../tools/cjing_build/premake/options.lua")
dofile("../../tools/cjing_build/premake/globals.lua")

solution "fcpp_sln"
	location ("build/" .. platform_dir ) 
	configurations { "Debug", "Release" }
	setup_project_env()
	
-- Project	
project "fcpp"
	location ("build\\" .. platform_dir)
	kind "StaticLib"
	objdir("build\\" .. platform_dir .. "\\Temp")
	language "C++"

	if _ACTION == "vs2017" or _ACTION == "vs2015" or ACTION == "vs2019" then
		systemversion(windows_sdk_version())
	end

	-- includedirs
	includedirs { "src\\" }
			
	-- Files
	files 
	{ 
		"src\\**.c",
		"src\\**.h",
	}

	-- ignore main.c
	removefiles 
	{ 
		"src\\usecpp.c",
	}
				
    filter {"configurations:Debug"}
		defines { "DEBUG" }
		entrypoint "WinMainCRTStartup"
		symbols "On"
		targetdir ("lib/" .. platform_dir)
		targetname "fcpp_d"
 
	filter {"configurations:Release"}
		defines { "NDEBUG" }
		entrypoint "WinMainCRTStartup"
		optimize "Speed"
		targetdir ("lib/" .. platform_dir)
		targetname "fcpp"
