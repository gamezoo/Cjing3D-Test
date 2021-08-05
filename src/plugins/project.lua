
-- plugin projects

group "plugins"

-----------------------------------------------------------------
-- IMGuiRhi
-----------------------------------------------------------------
-- Imgui rhi
create_plugin(
    "imguiRhi",
    { PROJECT_RENDERER_NAME }, nil,
    function(SOURCE_DIR)
        -- Files
        files 
        { 
            "../../3rdparty/imgui/**.h",
            "../../3rdparty/imgui/**.cpp"
        }
        
        vpaths { 
            [""] =  {
                SOURCE_DIR .. "/**.c",
                SOURCE_DIR .. "/**.cpp",
                SOURCE_DIR .. "/**.hpp",
                SOURCE_DIR .. "/**.h",
                SOURCE_DIR .. "/**.inl",
            },
            ["imgui"] =  {
                "../../3rdparty/imgui/**.h", 
                "../../3rdparty/imgui/**.cpp"
            }
        }

        -- includes
        includedirs {
            -- 3rdParty
            "../../3rdparty", 
        }
        
        -- Debug config
        filter {"configurations:Debug"}
            targetdir ("lib/" .. platform_dir .. "/Debug")
            defines { "DEBUG" }

        -- Release config
        filter {"configurations:Release"}
            targetdir ("lib/" .. platform_dir .. "/Release")
            defines { "NDEBUG" }
        filter { }
    end 
)

-----------------------------------------------------------------
-- Modules
-----------------------------------------------------------------
-- luaScripts
create_plugin(
    "luaScripts",
    { PROJECT_CORE_NAME, PROJECT_LUA_HELPER_NAME}, nil,
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
    { PROJECT_RESOURCE_NAME, PROJECT_GPU_NAME, PROJECT_RENDERER_NAME }, {"imguiRhi"},
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
        filter {}
    end 
)

group ""