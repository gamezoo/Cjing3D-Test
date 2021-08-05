
print("Start to build engine modulers")
-- Engine plugins
print("-------------------------------------------------------------")
print("[Plugins]")
print("-------------------------------------------------------------")
dofile "plugins/project.lua"
print("-------------------------------------------------------------")
-- engine modules
group "engine"
print("-------------------------------------------------------------")
print("[Engine modulers]")
print("-------------------------------------------------------------")
dofile "math/project.lua"
dofile "luaHelper/project.lua"
dofile "core/project.lua"
dofile "resource/project.lua"
dofile "gpu/project.lua"
dofile "renderer/project.lua"
dofile "network/project.lua"
-- client
dofile "client/project.lua"
group ""
