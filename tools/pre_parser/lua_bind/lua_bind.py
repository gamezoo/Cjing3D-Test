import json
import sys
import os
import traceback
import glob
import subprocess
import code_gen_utils.code_gen_utils as cgu

filter_path = "filter.json"

class ArgsInfo:
    input_files = []
    output_dir = ""
    input_dir  = ""

class MetaInfo:
    input_head_files = []
    class_base_mapping = dict()
    class_meta_infos = list()
    
    def is_empty(self):
        return len(self.class_meta_infos) <= 0

    def sort_dependencies(self):
        queue = list()
        in_map = dict()
        edge_map = dict()
        
        need_sort = False
        class_map = dict()
        # refresh class base mapping
        for class_meta_info in self.class_meta_infos:
            base_classes = class_meta_info["base_class"]
            class_name = class_meta_info["name"]
            if base_classes:
                class_base_info = self.class_base_mapping[class_name]
                for base_class in base_classes:
                    if base_class not in self.class_base_mapping.keys():
                        continue
                    class_base_info.append(base_class)

                    # set edge map
                    if not base_class in edge_map.keys():
                        edge_map[base_class] = list()
                    edge_map[base_class].append(class_name)
                    need_sort = True

                in_map[class_name] = len(base_classes)
            else:
                queue.append(class_name)
                in_map[class_name] = 0
            class_map[class_name] = class_meta_info
                
        if not need_sort:
            return

        # topological sort
        new_queue = list()
        while len(queue) > 0:
            class_name = queue[len(queue) - 1]
            queue.pop()
            new_queue.append(class_name)

            if class_name in edge_map.keys():
                for derived_class_name in edge_map[class_name]:
                    in_map[derived_class_name] = in_map[derived_class_name] - 1
                    if in_map[derived_class_name] == 0:
                        queue.append(derived_class_name)

        new_list = list()
        for class_name in new_queue:
            new_list.append(class_map[class_name])
        self.class_meta_infos = new_list 

def get_path_file_name(dir):
    return os.path.basename(dir)

def change_path_extension(dir, ext):
    return os.path.splitext(dir)[0] + ext

def check_path_directory(dir):
    return len(os.path.splitext(dir)[1]) <= 0

#########################################################################################################
# parse
#########################################################################################################

def print_help():
    print("lua_bind_parser v0.1.0")
    print("usage: lua_bind [option] [cmd] [params]")
    print("cmd arguments:")
    print("-help: display help")
    print("-i   : list of input files")
    print("-d   : input directory")
    print("-o   : ouput directory")

def parse_args(args):
    args_info = ArgsInfo()
    if len(args) == 1:
        print_help()
        return args_info
    
    for i in range(1, len(args)):
        if args[i] == "-i":
            file_index = i + 1
            while file_index < len(args) and args[file_index][0] != "-":
                args_info.input_files.append(args[file_index])
                file_index = file_index + 1
            i = file_index

        elif args[i] == "-o":
            args_info.output_dir = args[i + 1]

        elif args[i] == "-d":
            args_info.input_dir = args[i + 1]

    return args_info

def parse_jsn_meta_info(jsn, meta_info):
    if "head_name" in jsn.keys():
        meta_info.input_head_files.append(jsn["head_name"])

    if "class" in jsn.keys():
        class_meta_infos = jsn["class"]
        for i in range(0, len(class_meta_infos)):
            class_meta_info = class_meta_infos[i]

            meta_info.class_meta_infos.append(class_meta_info)
            meta_info.class_base_mapping[class_meta_info["name"]] = list()

def parse_meta_info(input_file_name, meta_info):
    # 0. make temp dir
    if not os.path.exists("./temp"):
        os.makedirs("./temp")

    # 1. use cppparser to generate meta.json
    cmd = "..\cppParser.exe"
    cmd += " -i " + input_file_name
    cmd += " -f " + filter_path
    cmd += " -od .\\temp"
    cmd += " -on temp.json"
    subprocess.call(cmd, shell=True)

    # 2. parse meta.json to generate lua_register code
    temp_path = "./temp/temp.json"
    if not os.path.exists(temp_path):
        return
    buffer = open(temp_path).read()
    if not buffer:
        return
    try:
        jsn = json.loads(buffer)
    except:
        traceback.print_exc()
        exit(1)
    if not jsn or len(jsn) <= 0:
        return
    parse_jsn_meta_info(jsn, meta_info)

    # 3. remove temp json file
    os.remove(temp_path)

def get_class_register_name(jsn):
    name = jsn["name"]
    attributes = jsn["attributes"]
    if attributes:
        for i in range(0, len(attributes)):
            attribute = attributes[i]
            if attribute.find("LUA_BINDER_NAME") != -1:
                npos = attribute.find("(")
                epos = attribute.find(")")
                name = attribute[npos + 1:epos]
                break
    return name

#########################################################################################################
# generate
#########################################################################################################

def generate_class_contruct(jsn):
    code = ".AddConstructor(_LUA_ARGS_("
    # params
    params = jsn["params"]
    if params:
        for i in range(0, len(params)):
            param = params[i]
            if i > 0:
                code += " ,"
            if param["is_const"]:
                code += "const "
            code += param["type"]
    code += "))"
    return cgu.src_line(code)

def generate_class_function(jsn, class_name):
    function_name = jsn["name"] 
    function_str = "&" + class_name + "::" + function_name

    # static function
    if jsn["is_static"]:
        code = cgu.src_line(".AddFunction(\"" + function_name+ "\", " + function_str + ")")
    else:
        code = cgu.src_line(".AddMethod(\"" + function_name+ "\", " + function_str + ")")
    return code

def generate_class_meta(mete_info, jsn):
    code = ""
    class_name = jsn["name"]
    
    # base classf
    class_base_mapping = mete_info.class_base_mapping[class_name]
    if len(class_base_mapping) > 0:
        for base_class_name in class_base_mapping:
            # multiple inheritance is not supported
            string = ".BeginExtendClass<"
            string += class_name + ", " + base_class_name + ">"
            string += "(\"" + get_class_register_name(jsn) + "\")"
            code += cgu.src_line(string)
            break
    else:
        code += cgu.src_line(".BeginClass<" + class_name + ">(\"" + get_class_register_name(jsn) + "\")")

    # construct
    constuctors = jsn["constuctors"]
    if constuctors:
        for constuctor in constuctors:
            code += generate_class_contruct(constuctor)

    # function 
    functions = jsn["member_function"]
    if functions:
        for function in functions:
            code += generate_class_function(function, class_name)

    code += cgu.src_line(".EndClass();")
    return code

def generate(meta_info, output_file_name):
    print("start to generate:", output_file_name)
 
    # 1. generate include heads codes
    code = cgu.src_line("// codegen_lua_bind")
    code += cgu.src_line("#pragma once")
    code += cgu.src_line("")
    code += cgu.src_line('#include ' + cgu.in_quotes("luaHelper\luaBinder.h"))
    for input_head in meta_info.input_head_files:
        code += cgu.src_line('#include ' + cgu.in_quotes(input_head))
    code += cgu.src_line("")

    # 2. sort classes by dependencies
    meta_info.sort_dependencies()

    # 3. generate lua registering codes
    code += cgu.src_line("using namespace Cjing3D;")
    code += cgu.src_line("")
    code += cgu.src_line("void luabind_registers_AutoBindFunction(lua_State* l) {")
    for i in range(0, len(meta_info.class_meta_infos)):
        class_jsn = meta_info.class_meta_infos[i]
        code += cgu.src_line("LuaBinder(l)")
        code += generate_class_meta(meta_info, class_jsn)
        code += cgu.src_line("")
    code += cgu.src_line("}")

    # 4. write file
    output_file = open(output_file_name, "w")
    output_file.write(cgu.format_source(code, 4))

#########################################################################################################
# main
#########################################################################################################

def run(args):
    args_info = parse_args(args)

    output_dir = args_info.output_dir
    if not output_dir or not check_path_directory(output_dir):
        return
    
    input_dir = args_info.input_dir
    if input_dir and check_path_directory(input_dir):
            ret = glob.glob(os.path.join(input_dir, "**/*.h"), recursive=True)
            if len(ret) > 0:
                for path in ret:
                    args_info.input_files.append(path)

    meta_info = MetaInfo()
    # parse input file
    for i in args_info.input_files:
        if not os.path.exists(i):
            continue
        parse_meta_info(i, meta_info)
    
    # generate lua bind codes
    if not meta_info.is_empty():
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        generate(meta_info, os.path.join(output_dir, "lua_bind_generated.h"))

if __name__ == '__main__':
    run(sys.argv)