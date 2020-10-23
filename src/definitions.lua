-- definitions
PROJECT_LUA_BINDER_NAME = "luaBinder"
PROJECT_MATH_NAME       = "math"
PROJECT_CORE_NAME       = "core"
PROJECT_RENDERER_NAME   = "renderer"
PROJECT_CLIENT_NAME     = "client"

all_project_table = 
{
    PROJECT_MATH_NAME,
    PROJECT_LUA_BINDER_NAME,
    PROJECT_CORE_NAME,
    PROJECT_CLIENT_NAME,
}

dependencies_mapping = 
{
    [PROJECT_CORE_NAME]   = { 
        PROJECT_MATH_NAME 
    },

    [PROJECT_CLIENT_NAME] = { 
        PROJECT_MATH_NAME, 
        PROJECT_CORE_NAME 
    },
}

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

function setup_dependencies(project_name)
    local dependencies = dependencies_mapping[project_name]
    if dependencies ~= nil then 
        for _, dependency in ipairs(dependencies) do
            dependson { dependency }
        end 
    end 
end 

function setup_dependent_libs(project_name, config)
    local dependencies = dependencies_mapping[project_name]
    if dependencies ~= nil then 
        for _, dependency in ipairs(dependencies) do
            libdirs {"../" .. dependency .. "/lib/" .. config}
            links {dependency}
            includedirs {"../" .. dependency .. "/src"}
        end 
    end 
end 

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

function setup_all_dependencies()
    for _, project_name in ipairs(all_project_table) do 
        dependson { project_name }
    end 
end 

function setup_engine_libs(config)
    -- lib dirs
    for _, project_name in ipairs(all_project_table) do 
        libdirs {env_dir .. "src/" .. project_name .. "/lib/" .. config}
    end 

    -- lib links
    for _, project_name in ipairs(all_project_table) do 
        links {project_name}
    end
end 

function setup_engine(config)
    -- include
    for _, project_name in ipairs(all_project_table) do 
        includedirs {env_dir .. "src/" .. project_name .. "/src"}
    end 

    -- libs
    setup_engine_libs(config)

    -- dependencies
    setup_all_dependencies()
end 