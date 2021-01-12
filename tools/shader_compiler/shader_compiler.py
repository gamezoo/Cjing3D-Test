import sys
import os.path
import threading
import subprocess

##########################################################################################
# Global Definitions
##########################################################################################

class GlobalInfo:
    shader_platform = ""  
    shader_version = ""   
    compiler_dir = ""
    tools_dir = ""
    inputs = []     
    entry_points = []
    output_dir = ""  
    shader_types = []

class CompileShaderInfo:
    name = ""
    source = ""
    entry_point = ""
    shader_type = ""    #(vs, gs, hs, ds, ps, cs)
    error_code = 0      # error code
    error_list = []     # list of errors / warnings
    output_list = []    # list of output


def parse_args():
    global global_info
    
    if len(sys.argv) == 1:
        display_help()
        return

    for i in range(1, len(sys.argv)):
        if "-help" in sys.argv[i]:
            display_help()
        if "-shader_platform" in sys.argv[i]:
            global_info.shader_platform = sys.argv[i + 1]
        if "-shader_version" in sys.argv[i]:
            global_info.shader_version = sys.argv[i + 1]
        if sys.argv[i] == "-i":
            j = i + 1
            while j < len(sys.argv) and sys.argv[j][0] != "-":
                global_info.inputs.append(sys.argv[j])
                j = j + 1
            if j >= len(sys.argv):
                break
            i = j
        if sys.argv[i] == "-o":
            global_info.output_dir = sys.argv[i + 1]
        if sys.argv[i] == "-e":
            j = i + 1
            while j < len(sys.argv) and sys.argv[j][0] != "-":
                global_info.entry_points.append(sys.argv[j])
                j = j + 1
            if j >= len(sys.argv):
                break
            i = j
        if sys.argv[i] == "-t":
            j = i + 1
            while j < len(sys.argv) and sys.argv[j][0] != "-":
                global_info.shader_types.append(sys.argv[j])
                j = j + 1
            if j >= len(sys.argv):
                break
            i = j

def call_wait_subprocess(cmdline):
    exclude_output = [
        "Microsoft (R)",
        "Copyright (C)",
        "compilation object save succeeded;"
    ]
    p = subprocess.Popen(cmdline, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    error_code = p.wait()
    output, err = p.communicate()
    err_str = err.decode('utf-8')
    err_str = err_str.strip(" ")
    err_list = err_str.split("\n")
    out_str = output.decode('utf-8')
    out_str = out_str.strip(" ")
    out_list = out_str.split("\n")

    clean_err = []
    for e in err_list:
        if len(e) > 0:
            clean_err.append(e.strip())

    clean_out = []
    for o in out_list:
        o = o.strip()
        exclude = False
        for ex in exclude_output:
            if o.startswith(ex):
                exclude = True
                break
        if len(o) > 0 and not exclude:
            clean_out.append(o)

    return error_code, clean_err, clean_out

##########################################################################################
# Compiling Functions
##########################################################################################
def compile_hlsl(global_info, shader_info):

    sm = str(global_info.shader_version)
    shader_model = {
        "vs": "vs_" + sm,
        "ps": "ps_" + sm,
        "gs": "gs_" + sm,
        "hs": "hs_" + sm,
        "ds": "ds_" + sm,
        "cs": "cs_" + sm,
    }
    exe = os.path.join(global_info.tools_dir, "hlsl", "dxc")

    cmdline = exe + " " + shader_info.source
    cmdline += " -T " + shader_model[shader_info.shader_type] + " "
    cmdline += " -E " + shader_info.entry_point + " "

    # config
    cmdline += " -pack-optimized "
    cmdline += " -res-may-alias "
    cmdline += " -no-legacy-cbuf-layout "
    cmdline += " -flegacy-macro-expansion "
    cmdline += " -D HLSL6 "

    # output
    output_name = shader_info.name + ".cso "
    output_path = global_info.output_dir
    #os.makedirs(output_path, exist_ok=True)
    output_file_and_path = os.path.join(output_path, shader_info.name + ".cso")

    # process cmd
    cmdline += " -Fo " + output_file_and_path
    error_code, error_list, output_list = call_wait_subprocess(cmdline)
    if error_code != 0:
        shader_info.error_code = error_code
        shader_info.error_list = error_list
        shader_info.output_list = output_list

##########################################################################################
# Main Functions
##########################################################################################
def display_help():
    print("cmd args:")
    print("    -shader_platform <hlsl, glsl>")
    print("    -shader_version")
    print("        hlsl: 6_5")
    print("        glsl: 330")
    print("    -i <list of input files or directories>")
    print("    -o <output dir for shaders>")
    print("    -e (optional) <list of entry point name, corresponds to -i>")
    print("    -t <list of shader type, corresponds to -i>")
    print("        hlsl: vs, gs, ds, hs, ps, cs")

def compile_shader_file(shader_info):
    if global_info.shader_platform == "hlsl":
        compile_hlsl(global_info, shader_info)
    else:
        print("error: invalid shader platform " + global_info.shader_platform)

def compile_shader_files(shader_infos):
    threads = []
    for shader_info in shader_infos:
        x = threading.Thread(target=compile_shader_file, args=(shader_info,))
        threads.append(x)
        x.start()

    for t in threads:
        t.join()

    for i in range(0, len(shader_infos)):
        shader_info = shader_infos[i]

        if shader_info.error_code == 0:
            print("compile successful:", shader_info.name)
        else:
            print(shader_info.name + " failed to compile")
        for out in shader_info.output_list:
            print(out)
        for err in shader_info.error_list:
            print(err)

def main():
    global global_info
    global_info = GlobalInfo()
    parse_args()

    source_list = global_info.inputs
    if len(source_list) <= 0:
        exit(0)

    #check 
    if len(global_info.entry_points) > 0 and len(source_list) != len(global_info.entry_points):
        print("error: the num of input files and entry points must be the same")
        exit(0)
    if len(source_list) != len(global_info.shader_types):
        print("error: the num of input files and shader types must be the same")
        exit(0)
    
    # set dirs
    file_dir = os.path.realpath(__file__)
    global_info.compiler_dir = os.path.dirname(file_dir)
    global_info.tools_dir = global_info.compiler_dir

    # compile inputs
    compile_shader_infos = []
    for index in range(0, len(source_list)):
        source = source_list[index]
        if os.path.isdir(source):
            print("error: input dir is not avaiable now.")
            exit(0)
            # for root, dirs, files in os.walk(source):
            #     for file in files:
            #         if file.endswith(".shdraw"):
            #             try:
            #                 compile_shader_file(file, root)
            #             except Exception as e:
            #                 print("ERROR: while processing", os.path.join(root, file))
            #                 raise e
        else:
            if not os.path.exists(source):
                print("error: file is not exists:", source)
                exit(0)

            shader_info = CompileShaderInfo()
            shader_info.name = os.path.basename(source).replace(".shdraw", "")
            shader_info.source = source
            shader_info.shader_type = global_info.shader_types[index]

            if index < len(global_info.entry_points):
                shader_info.entry_point = global_info.entry_points[index]
            else:
                shader_info.entry_point = "main"
            compile_shader_infos.append(shader_info)

    compile_shader_files(compile_shader_infos)


if __name__ == '__main__':
    print("------------------------------------------------------------")
    print("Shader compiler v0.1.0")
    print("------------------------------------------------------------")
    main()