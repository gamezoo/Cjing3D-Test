
import os
import platform
import glob

#################################################################
## common utils
#################################################################

def merge_dicts(dst, src):
    for k, v in src.items():
        if type(v) == dict:
            if k not in dst:
                dst[k] = v
        else:
            dst[k] = v
         

#################################################################
## platform utils
#################################################################
support_platforms = {
    "Windows" : "win32",
    "Linux" : "linux",
}

def get_platform_name():
    platform_name = platform.system()
    return support_platforms[platform_name]

def locate_winsdk_version_list():
    winsdk_dir = ""
    winsdk_name = "Windows Kits"
    programfiles = [ "PROGRAMFILES", "PROGRAMFILES(X86)"]
    for programfile in programfiles:
        env_dir = os.environ[programfile]
        if env_dir:
            if winsdk_name in os.listdir(env_dir):
                winsdk_dir = os.path.join(env_dir, winsdk_name)
                break

    if winsdk_dir:
        for version in os.listdir(winsdk_dir):
            if version == "10":
                child_dir = os.path.join(winsdk_dir, version, "Source")
                if not os.path.exists(child_dir):
                    continue
                return sorted(os.listdir(child_dir), reverse=False)
    return None

def path_replace_os_sep(path):
    path = path.replace("/", os.sep)
    path = path.replace("\\", os.sep)
    return path

#################################################################
## vs utils
#################################################################
support_vs_version = [
    "vs2019"
]

def check_vs_version(vs_version):
    if vs_version == "latest":
        return True
    return vs_version in support_vs_version

def locate_laste_vs_version():
    vs_root_dir = locate_vs_root()
    if len(vs_root_dir) <= 0:
        return ""

    ret = ""
    versions = sorted(os.listdir(vs_root_dir), reverse=False)
    for v in versions:
        vs_version = "vs" + v
        if vs_version in support_vs_version:
            ret = vs_version
    return ret

def locate_vs_root():
    vs_root = ""
    vs_directory_name = "Microsoft Visual Studio"
    programfiles = [ "PROGRAMFILES", "PROGRAMFILES(X86)"]
    for programfile in programfiles:
        env_dir = os.environ[programfile]
        if env_dir:
            if vs_directory_name in os.listdir(env_dir):
                vs_root = os.path.join(env_dir, vs_directory_name)
                print(vs_root)
    return vs_root
    

def locate_vcvarall():
    vs_root_dir = locate_vs_root()
    if len(vs_root_dir) <= 0:
        return None
    
    # **匹配所有文件、目录
    # recursive=True 递归匹配
    ret = glob.glob(os.path.join(vs_root_dir, "**/vcvarsall.bat"), recursive=True)
    if len(ret) > 0:
        return ret[0]
    return None


if __name__ == '__main__':
    print("utils")