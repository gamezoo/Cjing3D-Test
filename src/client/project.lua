
print("[Engine moduler] Client")

local function setup_platform_win32()
    systemversion(windows_sdk_version())
end 

local function setup_platform()
    if platform_dir == "win32" then 
        setup_platform_win32()
    end 
end 

project (PROJECT_CLIENT_NAME)
    location("build/" ..  platform_dir)
    objdir("build/" ..  platform_dir .. "/temp")
    kind "StaticLib"
    language "C++"
    conformanceMode(true)
    setup_project_env()
    setup_platform()
    setup_project_definines()
    setup_plugins_definines()
    targetname(PROJECT_CLIENT_NAME)

    -- dependencies
    setup_dependencies(PROJECT_CLIENT_NAME)

    -- Files
    local SOURCE_DIR = "src"
	files 
	{ 
        SOURCE_DIR .. "/**.c",
		SOURCE_DIR .. "/**.cpp",
        SOURCE_DIR .. "/**.hpp",
        SOURCE_DIR .. "/**.h",
        SOURCE_DIR .. "/**.inl",
    }
    
    -- includes
    includedirs {
        -- local
        SOURCE_DIR,
        
        -- 3rdParty
        "../../3rdparty", 
    }
    
    -- libdirs

    -- Debug config
    filter {"configurations:Debug"}
        targetdir ("lib/" .. platform_dir .. "/Debug")
        defines { "DEBUG" }
        setup_dependent_libs(PROJECT_CLIENT_NAME, "Debug")

    -- Release config
    filter {"configurations:Release"}
        targetdir ("lib/" .. platform_dir .. "/Release")
        defines { "NDEBUG" }
        setup_dependent_libs(PROJECT_CLIENT_NAME, "Release")