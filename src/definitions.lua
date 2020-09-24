-- definitions
PROJECT_MATH_NAME = "Math"


-- dependencies


dependencies_mapping = 
{


}

function setup_dependencies(project_name)
    local dependencies = dependencies_mapping[project_name]
    if dependencies ~= nil then 
        for dependency in iparis(dependencies) do
            dependson { dependency }
        end 
    end 
end 

function setup_all_dependencies()
    -- dependson {
    --     PROJECT_MATH_NAME,
    -- }
end 

function setup_all_libdirs()

end 

function setup_all_links()

end 

function setup_engine_libs(config)
    setup_all_libdirs()
    setup_all_links()
end 