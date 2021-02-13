
print("[Engine moduler] ImGUI")

local function setup_platform_win32()
    systemversion(windows_sdk_version())
end 

local function setup_platform()
    if platform_dir == "win32" then 
        setup_platform_win32()
    end 
end 

project (PROJECT_IMGUI)
    location("build/" ..  platform_dir)
    objdir("build/" ..  platform_dir .. "/temp")
    kind "StaticLib"
    language "C++"
    conformanceMode(true)
    setup_project_env()
    setup_platform()
    setup_project_definines()
    targetname(PROJECT_IMGUI)

    -- dependencies
    setup_dependencies(PROJECT_IMGUI)

    -- Files
    local SOURCE_DIR = "src"
	files 
	{ 
        SOURCE_DIR .. "/**.c",
		SOURCE_DIR .. "/**.cpp",
        SOURCE_DIR .. "/**.hpp",
        SOURCE_DIR .. "/**.h",
        SOURCE_DIR .. "/**.inl",

        "../../3rdparty/imgui/**.h",
        "../../3rdparty/imgui/**.cpp"
    }
    
    vpaths { 
        [""] =  {
            SOURCE_DIR .. "/**.c",
            SOURCE_DIR .. "/**.cpp",
            SOURCE_DIR .. "/**.hpp",
            SOURCE_DIR .. "/**.h",
            SOURCE_DIR .. "/**.inl",
        },
        ["imgui"] =  {
            "../../3rdparty/imgui/**.h", 
            "../../3rdparty/imgui/**.cpp"
        }
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
        setup_dependent_libs(PROJECT_IMGUI, "Debug")

    -- Release config
    filter {"configurations:Release"}
        targetdir ("lib/" .. platform_dir .. "/Release")
        defines { "NDEBUG" }
        setup_dependent_libs(PROJECT_IMGUI, "Release")
    filter { }