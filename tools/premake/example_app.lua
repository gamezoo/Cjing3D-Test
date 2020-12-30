local function setup_platform_win32()
    systemversion(windows_sdk_version())
end 

local function setup_platform()
    if platform_dir == "win32" then 
        setup_platform_win32()
    end 
end 

local function setup_third_modules()
end 

local function link_all_plugins(config)
    includedirs {env_dir .. "src/plugins/src"}

    for _, plugin in ipairs(all_plugins) do
        link_plugin(plugin)
    end
end 

----------------------------------------------------------------------------

function get_current_script_path()
    local str = debug.getinfo(2, "S").source:sub(2)
    return str:match("(.*/)")
end

function create_example_app(project_name, source_directory, root_directory, app_kind)
    print("[APP]", project_name)

    local project_dir = root_directory .. "/build/" .. platform_dir .. "/" .. project_name
    local target_dir  = root_directory .. "/bin/" .. platform_dir
    local source_dir  = root_directory .. source_directory

    project (project_name)
        location(project_dir)
        objdir(project_dir .. "/temp")
        cppdialect "C++17"
        language "C++"
        conformanceMode(true)
        kind (app_kind)
        staticruntime "Off"
        setup_project_env()
        setup_platform()
        setup_project_definines()
        setup_plugins_definines()

        -- includes
        includedirs { 
            -- local
            project_dir,

            -- 3rdParty
            env_dir .. "3rdparty/",
        }

        -- 3rd library
        setup_third_modules()

        -- Files
        files 
        { 
            source_dir .. "/**.c",
            source_dir .. "/**.cpp",
            source_dir .. "/**.hpp",
            source_dir .. "/**.h",
            source_dir .. "/**.inl"
        }

        -- Debug dir
        local debug_dir = ""
        if work_dir ~= nil then 
            debug_dir = env_dir .. work_dir
        else
            debug_dir = env_dir .. "assets"
        end 
        debugdir (debug_dir)

        --------------------------------------------------------------
        -- Config
        targetdir (target_dir)
        -- Debug config
        filter {"configurations:Debug"}
            targetname(project_name)
            defines { "DEBUG" }
            setup_engine("Debug")
            link_all_plugins("Debug")

        -- Release config
        filter {"configurations:Release"}
            targetname(project_name .. "_d")
            defines { "NDEBUG" }
            setup_engine("Release")
            link_all_plugins("Release")
        filter { }
        --------------------------------------------------------------
end