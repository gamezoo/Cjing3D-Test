
dofile "plugins.lua"

-----------------------------------------------------------------------
-- Plugins 
-----------------------------------------------------------------------
print("-------------------------------------------------------------")
print("[Plugins]")

all_plugins = {
    "resConverter"
}

-----------------------------------------------------------------------

group "plugins"
for _, plugin in ipairs(all_plugins) do
    create_plugin(
        plugin,
        { PROJECT_RESOURCE_NAME }
    )
end
group ""

print("-------------------------------------------------------------")
