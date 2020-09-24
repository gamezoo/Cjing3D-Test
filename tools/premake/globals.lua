-----------------------------------------------------------------------------------
-- custom api
-----------------------------------------------------------------------------------
require('vstudio')

premake.api.register {
    name = "conformanceMode",
    scope = "config",
    kind = "boolean",
    default = true
}

local function HandleConformanceMode(cfg)
    if cfg.conformanceMode ~= nil and cfg.conformanceMode == true then
        premake.w('<ConformanceMode>true</ConformanceMode>')
    end
end
  
if premake.vstudio ~= nil then 
    if premake.vstudio.vc2010 ~= nil then 
        premake.override(premake.vstudio.vc2010.elements, "clCompile", function(base, cfg)
            local calls = base(cfg)
            table.insert(calls, HandleConformanceMode)
            return calls
        end)
    end 
end 
-----------------------------------------------------------------------------------

function windows_sdk_version()
	if sdk_version ~= "" then
		return sdk_version
	end
	return "10.0.18362.0"
end

-----------------------------------------------------------------------------------
-- env
-----------------------------------------------------------------------------------
renderer = ""
platform_dir = ""
sdk_version = ""
env_dir = "../"
current_platform = "unknown"

function setup_project_env_win32()
    platforms "x64"
    filter { "platforms:x64" }
        system "Windows"
        architecture "x64"
end 

function setup_project_env_linux()
	print("Linux is unspported now.")
end 

function setup_project_env()
    if current_platform == "win32" then 
        setup_project_env_win32()
    elseif current_platform == "linux" then 
        setup_project_env_linux()
    end
end

function setup_project_definines()
    defines
    {
        ("CJING3D_PLATFORM_" .. string.upper(platform_dir)),
        ("CJING3D_RENDERER_" .. string.upper(renderer)),
    }
end 

-----------------------------------------------------------------------------------
-- main
-----------------------------------------------------------------------------------
function setup_env_from_options()
    if _OPTIONS["renderer"] then 
        renderer = _OPTIONS["renderer"]
    end 
    if _OPTIONS["platform_dir"] then
        platform_dir = _OPTIONS["platform_dir"]
    end 
    if _OPTIONS["env_dir"] then
        env_dir = _OPTIONS["env_dir"]
    end 
    if _OPTIONS["sdk_version"] then
        sdk_version = _OPTIONS["sdk_version"]
    end  
end

function setup_env_from_action()
    if _ACTION == "vs2017" or _ACTION == "vs2019" then
        current_platform = "win32" 
    end 

    print("[premake]:current_platform:", current_platform)
end 

setup_env_from_options()
setup_env_from_action()