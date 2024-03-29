dofile("../tools/cjing_build/premake/options.lua")
dofile("../tools/cjing_build/premake/globals.lua")
dofile("../tools/cjing_build/premake/plugins.lua")
dofile("../tools/cjing_build/premake/example_app.lua")
dofile("../src/definitions.lua")

editor_name = "editor"
app_name = "app"

start_project = editor_name
if not build_editor then 
    start_project = app_name
end 

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
print("-------------------------------------------------------------")
print("[APPS]")
print("-------------------------------------------------------------")
if build_editor then 
    create_example_app(
        editor_name, 
        "src/editor", 
        get_current_script_path(), 
        "WindowedApp", 
        all_plugins, 
        {PROJECT_IMGUI}
    )
end 

if build_app then 
    create_example_app(
        app_name, 
        "src/app", 
        get_current_script_path(), 
        "WindowedApp"
    )
end 

if build_tests then 
    create_example_app(
        "tests", 
        "src/tests", 
        get_current_script_path(), 
        "ConsoleApp",
        all_plugins
    )
end 
print("-------------------------------------------------------------")