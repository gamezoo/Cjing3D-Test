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

local function setup_plugins_definines()
    if #all_plugins <= 0 then 
        return
    end 

    local definines_str = ""
    for index, plugin in ipairs(all_plugins) do
        if index > 1 then 
            definines_str = definines_str .. ",";
        end
        definines_str = definines_str .. "\"" .. plugin .. "\""
    end
    defines { "CJING_PLUGINS=" .. definines_str }
end 

local function link_all_plugins(config)
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
    print("-------------------------------------------------------------")
    print("[APP]")
    print("-------------------------------------------------------------")

    local project_dir = root_directory .. "/build/" .. platform_dir
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

        --------------------------------------------------------------
        -- config
        targetdir (target_dir)
        debugdir (target_dir)

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
        --------------------------------------------------------------
end