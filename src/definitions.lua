-- definitions
PROJECT_LUA_HELPER_NAME = "luaHelper"
PROJECT_MATH_NAME       = "math"
PROJECT_CORE_NAME       = "core"
PROJECT_RENDERER_NAME   = "renderer"
PROJECT_CLIENT_NAME     = "client"
PROJECT_RESOURCE_NAME   = "resource"
PROJECT_GPU_NAME        = "gpu"
PROJECT_NETWORK         = "network"

-- default modules
register_engine_module(PROJECT_LUA_HELPER_NAME)
register_engine_module(PROJECT_MATH_NAME)
register_engine_module(
    PROJECT_CORE_NAME,
    { 
        PROJECT_MATH_NAME,
        PROJECT_LUA_HELPER_NAME
    }
)
register_engine_module(
    PROJECT_RESOURCE_NAME,
    { 
        PROJECT_MATH_NAME, 
        PROJECT_CORE_NAME 
    }
)
register_engine_module(
    PROJECT_GPU_NAME,
    { 
        PROJECT_MATH_NAME, 
        PROJECT_CORE_NAME 
    }
)
register_engine_module(
    PROJECT_RENDERER_NAME,
    { 
        PROJECT_GPU_NAME,
        PROJECT_RESOURCE_NAME
    }
)
register_engine_module(
    PROJECT_CLIENT_NAME,
    { 
        PROJECT_MATH_NAME, 
        PROJECT_CORE_NAME,
        PROJECT_RESOURCE_NAME,
        PROJECT_GPU_NAME,
        PROJECT_RENDERER_NAME
    }
)
-- extra modules
register_engine_module(
    PROJECT_NETWORK,
    {PROJECT_CORE_NAME},
    false
)