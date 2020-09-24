# CJBuild is based on pmtech's pmbuild
# https://github.com/polymonster/pmtech

import sys
import os.path
import json
import time
import shutil
import collections
import subprocess
import platform
import utils
import jsc.jsc as jsc

def get_tools(tool_name, config):
    tool = utils.path_replace_os_sep(config["tools"][tool_name])
    if platform.system() == "Windows":
        tool += ".exe"
    return tool

##########################################################################################
# Task Methods
##########################################################################################

def taks_run_libs(config):
    print("-----------------------------------------------------------------------")
    print("Task: build libs")
    print("-----------------------------------------------------------------------")
    
    for lib_cmd in config["libs"]:    
        args = config["env_dir"] + "/" + " "
        args += config["sdk_version"] + " "
        if "vs_version" not in config:
            config["vs_version"] = "vs2019"
        args += config["vs_version"] + " "
        final_cmd = lib_cmd + "\"" + config["vcvarsall_dir"] + "\"" + " " + args

        p = subprocess.Popen(final_cmd, shell=True)
        p.wait()

def task_run_premake(config):
    print("-----------------------------------------------------------------------")
    print("Task: Premake")
    print("-----------------------------------------------------------------------")
    premake_cmd = get_tools("premake", config)
    if not premake_cmd:
        print("error: cannot find tool:premake")
        exit(1)
    # 遍历添加所有config permake cmd
    for cmd in config["premake"]:
        if cmd == "vs_version":
            cmd = config["vs_version"]
        premake_cmd += " " + cmd
    # add env dir
    if "env_dir" in config.keys():
        premake_cmd += " --env_dir=\"" + config["env_dir"] + "\""
    # add sdk version
    if "sdk_version" in config.keys():
        premake_cmd += " --sdk_version=\"" + str(config["sdk_version"]) + "\""

    print(premake_cmd)
    subprocess.call(premake_cmd, shell=True)

def taks_run_copy(config):
    print("-----------------------------------------------------------------------")
    print("Task: Copy")
    print("-----------------------------------------------------------------------")

def task_run_build(config):
    print("-----------------------------------------------------------------------")
    print("Task: Build")
    print("-----------------------------------------------------------------------")

def task_run_clean(config):
    print("-----------------------------------------------------------------------")
    print("Task: Clean")
    print("-----------------------------------------------------------------------")

    for clean_task in config["clean"]:
        if os.path.isdir(clean_task):
            print("clean directory:" + clean_task)
            shutil.rmtree(clean_task)
        elif os.path.isfile(clean_task):
            print("clean file:" + clean_task)
            os.remove(clean_task)

def generate_cjingbuild_config(config, profile):
    pass

##########################################################################################

def check_vs_version(config):
    vs_version = config["vs_version"]
    if not utils.check_vs_version(vs_version):
        print("error: unsupported visual studio version:" + str(version))
        exit(1)
    if vs_version == "latest":
        config["vs_version"] = utils.locate_laste_vs_version()
        print("set vs version:" + config["vs_version"])

def set_user_config_update(key, value, config):
    config[key] = value

    user_cfg = dict()
    if os.path.exists("config_user.jsc"):
        user_cfg = json.loads(open("config_user.jsc", "r").read())
    user_cfg[key] = value

    file = open("config_user.jsc", "w+")
    file.write(json.dumps(user_cfg, indent=4))
    file.close()

# 
def set_user_config_winsdk_version(config, wanted_sdk_version):
    if "sdk_version" in config.keys():
        if not wanted_sdk_version or wanted_sdk_version == "latest":
            return

    sdk_version_list = utils.locate_winsdk_version_list()
    if sdk_version_list:
        sdk_version = ""
        if not wanted_sdk_version or wanted_sdk_version == "latest":
            sdk_version = sdk_version_list[0]
        elif wanted_sdk_version not in sdk_version_list:
            print("error:Windows SDK is not installed. version:" + wanted_sdk_version)
            exit(1)
        else:
            sdk_version = wanted_sdk_version

        if sdk_version:
            set_user_config_update("sdk_version", sdk_version, config)
    else:
        print("error:Cannot find one setted Windows SDK")
        print("error:Some task is invalid")
        exit(1)

# find vcvarsall.bat directory
def set_user_config_vc_vars(config):
    if "vcvarsall_dir" in config.keys():
        if os.path.exists(config["vcvarsall_dir"]):
            return

    vcvarsall_dir = utils.locate_vcvarall()
    if vcvarsall_dir:
        vcvarsall_dir = os.path.dirname(vcvarsall_dir)
        set_user_config_update("vcvarsall_dir", vcvarsall_dir, config)
        return
    else:
        print("error:Cannot find vcvarsall.bat")
        print("error:Some task is invalid")
        exit(1)
    
# 构建user config
def set_user_config(config):
    user_cfg = dict()
    if os.path.exists("config_user.jsc"):
        user_cfg = json.loads(open("config_user.jsc", "r").read())

    if utils.get_platform_name() == "win32":
        set_user_config_vc_vars(user_cfg)
        set_user_config_winsdk_version(user_cfg, config["sdk_version"])

    # 需要将user_config合并到config中
    if os.path.exists("config_user.jsc"):
        user_cfg = json.loads(open("config_user.jsc", "r").read())
        utils.merge_dicts(config, user_cfg)


def print_help(config):
    print("-----------------------------------------------------------------------")
    print("Cjing build system -help ")
    print("-----------------------------------------------------------------------")
    print("usage: pmbuild <options> <profile> <tasks...>")
    print("cmd arguments:")
    print("options:")
    print("      -help (display help)")
    print("      -show_cfg (print cfg json)")
    print("profile:(edit in config.jsc)")
    for p in config.keys():
        print(" " * 6 + p)
    print("task:")
    print("      -all ")
    print("      -libs")
    print("      -premake")
    print("      -copy")

def main():
    start_time = time.time()

    if not os.path.exists("config.jsc"):
        print("[error] must have a config.jsc ni current directory.")
        exit(1)
    
    config_jsc = jsc.parse_jsc(open("config.jsc", "r").read())
    if not config_jsc:
        print("[error] invalid config:config.jsc.")
        exit(1)

    # parse args
    #######################################
    # option
    option_index = 0
    if "-help" in sys.argv:
        print_help(config_jsc)
        option_index += 1

    if "-show_cfg" in sys.argv:
        print("cfg_json:")
        print(json.dumps(config_jsc, indent=4))
        option_index += 1

    if (len(sys.argv) <= option_index + 1):
        exit(1)

   #######################################
    # profile
    profile = sys.argv[option_index + 1]
    if profile not in config_jsc:
        print("[error] " + profile + " is not a valid profile")
        exit(0)     
    profile_cfg = config_jsc[profile]

    set_user_config(profile_cfg)

    # check vs version
    if "vs_version" in profile_cfg:
        check_vs_version(profile_cfg)

   #######################################
    # tasks
    tasks = collections.OrderedDict()
    tasks["libs"]    = { "run" : taks_run_libs, "exclusive" : True}
    tasks["premake"] = { "run" : task_run_premake}
    tasks["copy"]    = { "run" : taks_run_copy}
    tasks["build"]   = { "run" : task_run_build}

    # clean总是最先执行，所以独立出来优先处理
    if "-clean" in sys.argv:
        task_run_clean(profile_cfg)

    for key in tasks.keys():
        if key not in profile_cfg.keys():
            continue
        task_start_time = time.time()
        run_task = False

        if "-all" in sys.argv:
            if "exclusive" in tasks[key].keys() and tasks[key]["exclusive"]:
                continue
            run_task = True
        elif "-" + key in sys.argv:
            run_task = True

        if run_task:
            tasks.get(key, )["run"](profile_cfg)
            print("Took (" + str(int((time.time() - task_start_time) * 1000)) + "ms)")
            print("-----------------------------------------------------------------------")
            print("")

    generate_cjingbuild_config(profile_cfg, profile)

    # finish jobs
    print("***********************************************************************")
    print("Finish all jobs. Took (" + str(int((time.time() - start_time) * 1000)) + "ms)")
    print("***********************************************************************")

if __name__ == '__main__':
    print("***********************************************************************")
    print("Cjing Build system v0.1.0")
    print("***********************************************************************")
    main()
