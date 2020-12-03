
dofile "plugins/definitions.lua"

print("Start to build engine modulers")
-- engine modules
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
-- Engine plugins
print("-------------------------------------------------------------")
print("[Plugins]")
print("-------------------------------------------------------------")
dofile "plugins/project.lua"
print("-------------------------------------------------------------")