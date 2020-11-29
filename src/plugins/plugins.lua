
local function setup_platform_win32()
    systemversion(windows_sdk_version())
end 

local function setup_platform()
    if platform_dir == "win32" then 
        setup_platform_win32()
    end 
end 

local function set_plugin_env(plugin_dependencies, config)
    if not plugin_dependencies then 
        return
    end 
    
    local dependency_map = {}
    
    for _, dependency in ipairs(plugin_dependencies) do 
        get_all_dependencies(dependency, dependency_map)
    end 

    for dependency, v in pairs(dependency_map) do 
        libdirs {"../" .. dependency .. "/lib/" .. config}
        links {dependency}
        includedirs {"../" .. dependency .. "/src"}
    end 
end 

function link_plugin(plugin_name)
    links(plugin_name)
    if is_static_plugin then 
        force_Link ("static_plugin_" .. plugin_name)
    end 
end 

function create_plugin(plugin_name, plugin_dependencies)
    print("-------------------------------------------------------------")
    print("[Plugin]", plugin_name)
    print("-------------------------------------------------------------")

    project (plugin_name)
    location("build/" ..  platform_dir)
    objdir("build/" ..  platform_dir .. "/temp")
    kind "StaticLib"
    language "C++"
    conformanceMode(true)
    setup_project_env()
    setup_platform()
    setup_project_definines()
    targetname(plugin_name)

    -- Files
    local SOURCE_DIR = "src/" .. plugin_name
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
    
    -- Debug config
    filter {"configurations:Debug"}
        targetdir ("lib/" .. platform_dir .. "/Debug")
        defines { "DEBUG" }
        set_plugin_env(plugin_dependencies, "Debug")

    -- Release config
    filter {"configurations:Release"}
        targetdir ("lib/" .. platform_dir .. "/Release")
        defines { "NDEBUG" }
        set_plugin_env(plugin_dependencies, "Release")
end