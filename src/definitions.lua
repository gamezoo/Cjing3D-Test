-- definitions
PROJECT_MATH_NAME  = "Math"
PROJECT_LUA_BINDER_NAME = "LuaBinder"

all_project_table = 
{
    PROJECT_MATH_NAME,
    PROJECT_LUA_BINDER_NAME
}

dependencies_mapping = 
{
}

-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

function setup_dependencies(project_name)
    local dependencies = dependencies_mapping[project_name]
    if dependencies ~= nil then 
        for _, dependency in iparis(dependencies) do
            print("dependson", dependency)
            dependson { dependency }
        end 
    end 
end 

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