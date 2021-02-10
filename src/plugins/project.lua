
-- plugin projects

group "plugins"

-----------------------------------------------------------------
-- Converter
-----------------------------------------------------------------
group "plugins/converters"
-- plugin shader converter
create_plugin(
    "shaderConverter",
    { PROJECT_RESOURCE_NAME, PROJECT_GPU_NAME, PROJECT_RENDERER_NAME },
    function()
        -- includes
        includedirs {
            "../../assets/shaders", 
        }

        -- libdirs
        libdirs {  "../../3rdparty/fcpp/lib/" .. platform_dir }
        
        -- Debug config
        filter {"configurations:Debug"}
            links {"fcpp_d"}

        -- Release config
        filter {"configurations:Release"}
            links {"fcpp"}
    end 
)

create_plugin(
    "textureConverter",
    { PROJECT_RESOURCE_NAME, PROJECT_GPU_NAME, PROJECT_RENDERER_NAME }
)

create_plugin(
    "modelConverter",
    { PROJECT_RESOURCE_NAME, PROJECT_GPU_NAME, PROJECT_RENDERER_NAME }
)

create_plugin(
    "materialConverter",
    { PROJECT_RESOURCE_NAME, PROJECT_GPU_NAME, PROJECT_RENDERER_NAME }
)


group ""