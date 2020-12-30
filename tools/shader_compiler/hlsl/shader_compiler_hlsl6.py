import os
from subprocess import check_output

def compile_cmd(cmd):
    try:
        print(cmd)
        result = check_output(cmd, shell=True).decode()
        if len(result) > 0:
            print(result)
    except:
        print("DXC error")

def compile_shader(filename, profile, output_dir):
    cmd = "dxc " + name + " -T "
    
    if profile == "VS":
        cmd += "vs"
    if profile == "PS":
        cmd += "ps"
    if profile == "GS":
        cmd += "gs"
    if profile == "HS":
        cmd += "hs"
    if profile == "DS":
        cmd += "ds"
    if profile == "CS":
        cmd += "cs"
    if profile == "LIB":
        cmd += "lib"
    if profile == "MS":
        cmd += "ms"
    if profile == "AS":
        cmd += "as"

    cmd += "_6_5 "      
    cmd += " -flegacy-macro-expansion "  
    cmd += " -D HLSL6 "

    output_name = os.path.splitext(name)[0] + ".cso "
    cmd += " -Fo " + output_dir + "/" + output_name
    
    compile_cmd(cmd)

if __name__ == '__main__':
    print("***********************************************************************")
    print("Shader compiler hlsl6")
    print("***********************************************************************")
