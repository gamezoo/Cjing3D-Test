dofile("../../tools/premake/options.lua")
dofile("../../tools/premake/globals.lua")

solution "lua_build"
	location ("build/" .. platform_dir ) 
	configurations { "Debug", "Release" }
	setup_project_env()
	
-- Project	
project "lua"
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
		"src\\**.hpp",
	}

	-- ignore main.c
	removefiles 
	{ 
		"src\\lua.c",
		"src\\luac.c" 
	}
				
    filter {"configurations:Debug"}
		defines { "DEBUG" }
		entrypoint "WinMainCRTStartup"
		symbols "On"
		targetdir ("lib/" .. platform_dir)
		targetname "lua_lib_d"
 
	filter {"configurations:Release"}
		defines { "NDEBUG" }
		entrypoint "WinMainCRTStartup"
		optimize "Speed"
		targetdir ("lib/" .. platform_dir)
		targetname "lua_lib"
