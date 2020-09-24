dofile("../../tools/premake/options.lua")
dofile("../../tools/premake/globals.lua")
dofile("../../tools/premake/example_app.lua")
dofile("../../src/definitions.lua")

-- total solution
solution ("Cjing3D")
    location ("build/" .. platform_dir ) 
    cppdialect "C++17"
    language "C++"
    startproject "test"
    configurations { "Debug", "Release" }
    setup_project_env()

    -- Debug config
    filter {"configurations:Debug"}
        flags { "MultiProcessorCompile"}
        symbols "On"

    -- Release config
    filter {"configurations:Release"}
        flags { "MultiProcessorCompile"}
        optimize "On"

    -- Reset the filter for other settings
    filter { }
    
-- engine lib project
dofile "../src/project.lua"

-- example projects
print(get_current_script_path())
create_example_app("test", "src", get_current_script_path(), "ConsoleApp")