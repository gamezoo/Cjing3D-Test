
-- plugin projects

group "plugins"

-- plugin shader converter
create_plugin(
    "shaderConverter",
    { PROJECT_RESOURCE_NAME },
    function()
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

group ""