
print("[Engine moduler] Math")

local function setup_platform_win32()
    systemversion(windows_sdk_version())
end 

local function setup_platform()
    if platform_dir == "win32" then 
        setup_platform_win32()
    end 
end 

project (PROJECT_MATH_NAME)
    location("build/" ..  platform_dir)
    objdir("build/" ..  platform_dir .. "/temp")
    kind "StaticLib"
    language "C++"
    conformanceMode(true)
    setup_project_env()
    setup_platform()
    setup_project_definines()
    targetname(PROJECT_MATH_NAME)

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
    
    -- Includes
    includedirs {
        -- local
        SOURCE_DIR,
        
        -- 3rdParty
        "../../3rdparty", 
    }
    
    -- Debug config
    filter {"configurations:Debug"}
        targetdir ("lib/" .. platform_dir .. "/Debug")
        defines { "DEBUG" }

    -- Release config
    filter {"configurations:Release"}
        targetdir ("lib/" .. platform_dir .. "/Release")
        defines { "NDEBUG" }
    filter { }