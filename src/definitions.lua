-- definitions
PROJECT_LUA_HELPER_NAME = "luaHelper"
PROJECT_MATH_NAME       = "math"
PROJECT_CORE_NAME       = "core"
PROJECT_RENDERER_NAME   = "renderer"
PROJECT_CLIENT_NAME     = "client"
PROJECT_RESOURCE_NAME   = "resource"
PROJECT_GPU_NAME        = "gpu"
PROJECT_IMGUI           = "imguiRhi"
PROJECT_NETWORK         = "network"

default_engine_dependencies = 
{
    PROJECT_MATH_NAME,
    PROJECT_CORE_NAME,
    PROJECT_CLIENT_NAME,
    PROJECT_RESOURCE_NAME,
    PROJECT_GPU_NAME,
    PROJECT_RENDERER_NAME
}

dependencies_mapping = 
{
    [PROJECT_CORE_NAME]   = { 
        PROJECT_MATH_NAME,
        PROJECT_LUA_HELPER_NAME
    },

    [PROJECT_RESOURCE_NAME] = { 
        PROJECT_MATH_NAME, 
        PROJECT_CORE_NAME 
    },

    [PROJECT_CLIENT_NAME] = { 
        PROJECT_MATH_NAME, 
        PROJECT_CORE_NAME,
        PROJECT_RESOURCE_NAME,
        PROJECT_GPU_NAME,
        PROJECT_RENDERER_NAME
    },

    [PROJECT_GPU_NAME] = { 
        PROJECT_MATH_NAME, 
        PROJECT_CORE_NAME 
    },

    [PROJECT_RENDERER_NAME] = { 
        PROJECT_GPU_NAME,
        PROJECT_RESOURCE_NAME
    },

    [PROJECT_IMGUI] = { 
        PROJECT_RENDERER_NAME
    },

    [PROJECT_NETWORK] = { 
        PROJECT_CORE_NAME
    },
}

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

function get_all_dependencies(project_name, hash_map)
    hash_map[project_name] = 1

    local dependencies = dependencies_mapping[project_name]
    if dependencies == nil or #dependencies <= 0 then
        return
    end 
    
    for _, dependency in ipairs(dependencies) do
        get_all_dependencies(dependency, hash_map)
    end 
end 

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
        local dependency_map = {}
        for _, dependency in ipairs(dependencies) do 
            get_all_dependencies(dependency, dependency_map)
        end 
    
        for dependency, v in pairs(dependency_map) do 
            libdirs {"../" .. dependency .. "/lib/" .. config}
            links {dependency}
            includedirs {"../" .. dependency .. "/src"}
        end 
    end 
end 

function setup_engine(config, dependencies)
    -- include
    for _, project_name in ipairs(dependencies) do 
        includedirs {env_dir .. "src/" .. project_name .. "/src"}
    end 

    -- libs
    -- lib dirs
    for _, project_name in ipairs(dependencies) do 
        libdirs {env_dir .. "src/" .. project_name .. "/lib/" .. config}
    end 

    -- lib links
    for _, project_name in ipairs(dependencies) do 
        links {project_name}
    end
    

    -- dependencies
    for _, project_name in ipairs(dependencies) do 
        dependson { project_name }
    end 
end 