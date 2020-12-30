
print("[Engine moduler] GPU")

local function setup_platform_win32()
    systemversion(windows_sdk_version())
end 

local function setup_platform()
    if platform_dir == "win32" then 
        setup_platform_win32()
    end 
end 

local function set_current_renderer(src_path, renderer)
    if renderer == nil or renderer == "" then 
        return
    end 
    
    local path = src_path
    if renderer == "dx11" then 
        path = path .. "/dx11" 
    elseif renderer == "dx12" then 
        path = path .. "/dx12" 
    else
        print("[GPU] Invalid renderer", renderer)
        return
    end 

    files 
    { 
        path .. "/*.c",
        path .. "/*.cpp",
        path .. "/*.hpp",
        path .. "/*.h",
        path .. "/*.inl",
    }
end 

project (PROJECT_GPU_NAME)
    location("build/" ..  platform_dir)
    objdir("build/" ..  platform_dir .. "/temp")
    kind "StaticLib"
    language "C++"
    conformanceMode(true)
    setup_project_env()
    setup_platform()
    setup_project_definines()
    targetname(PROJECT_GPU_NAME)

    -- dependencies
    setup_dependencies(PROJECT_GPU_NAME)

    -- Files
    local SOURCE_DIR = "src/" .. PROJECT_GPU_NAME
	files 
	{ 
        SOURCE_DIR .. "/*.c",
		SOURCE_DIR .. "/*.cpp",
        SOURCE_DIR .. "/*.hpp",
        SOURCE_DIR .. "/*.h",
        SOURCE_DIR .. "/*.inl",
    }
    
    -- includes
    local include_dir = "src/"
    includedirs {
        -- local
        include_dir,
        
        -- 3rdParty
        "../../3rdparty", 
    }
    
    -- renderer
    set_current_renderer(SOURCE_DIR, renderer)

    -- Debug config
    filter {"configurations:Debug"}
        targetdir ("lib/" .. platform_dir .. "/Debug")
        defines { "DEBUG" }
        setup_dependent_libs(PROJECT_GPU_NAME, "Debug")

    -- Release config
    filter {"configurations:Release"}
        targetdir ("lib/" .. platform_dir .. "/Release")
        defines { "NDEBUG" }
        setup_dependent_libs(PROJECT_GPU_NAME, "Release")
    filter { }