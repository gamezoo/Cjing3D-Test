
-- plugin projects

group "plugins"

-----------------------------------------------------------------
-- Converter
-----------------------------------------------------------------
-- plugin shader converter
create_plugin(
    "resConverter",
    { PROJECT_RESOURCE_NAME, PROJECT_GPU_NAME, PROJECT_RENDERER_NAME },
    function()
        -- includes
        includedirs {
            "../../assets/shaders", 
        }

        -- libdirs
        libdirs {  "../../3rdparty/fcpp/lib/" .. platform_dir }
        
        -- todo, better way?
        local shaderInterop_path = path.getabsolute("../renderer/src/renderer")
        defines { "SHADER_INTEROP_PATH=\"" .. shaderInterop_path .. "\""}

        -- Debug config
        filter {"configurations:Debug"}
            links {"fcpp_d"}

        -- Release config
        filter {"configurations:Release"}
            links {"fcpp"}
    end 
)

group ""