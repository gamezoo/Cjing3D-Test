
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
dofile "luabinder/project.lua"
dofile "core/project.lua"
dofile "resource/project.lua"
dofile "client/project.lua"
dofile "gpu/project.lua"
dofile "renderer/project.lua"
dofile "imguiRhi/project.lua"
group ""
