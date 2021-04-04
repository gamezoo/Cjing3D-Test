
-- plugin projects

group "plugins"

-----------------------------------------------------------------
-- Modules
-----------------------------------------------------------------
-- luaScripts
create_plugin(
    "luaScripts",
    { PROJECT_CORE_NAME, PROJECT_LUA_HELPER_NAME},
    function()
        -- libdirs
        libdirs {  "../../3rdparty/lua/lib/" .. platform_dir,  }

        -- Debug config
        filter {"configurations:Debug"}
            links {"lua_lib_d"}

        -- Release config
        filter {"configurations:Release"}
            links {"lua_lib"}
        filter {}
    end 
)

-----------------------------------------------------------------
-- Converter
-----------------------------------------------------------------
-- plugin shader converter
create_plugin(
    "resConverter",
    { PROJECT_RESOURCE_NAME, PROJECT_GPU_NAME, PROJECT_RENDERER_NAME, PROJECT_IMGUI },
    function()
        -- includes
        includedirs {
            "../../assets/shaders", 
        }

        -- libdirs
        libdirs {  "../../3rdparty/fcpp/lib/" .. platform_dir }
        libdirs {  "../../3rdparty/nvtt/lib"}

        -- todo, better way?
        local shaderInterop_path = path.getabsolute("../renderer/src/renderer")
        defines { "SHADER_INTEROP_PATH=\"" .. shaderInterop_path .. "\""}

        -- Debug config
        filter {"configurations:Debug"}
            links {"fcpp_d"}
            links {"nvtt"}

        -- Release config
        filter {"configurations:Release"}
            links {"fcpp"}
            links {"nvtt"}
        filter {}
    end 
)

group ""