import sys
import time
import collections
import lua_bind.lua_bind as lua_bind

def task_run_lua_bind(argi):
    lua_bind.run(sys.argv[argi:])

def print_help():
    print("usage: parser [option] [task] [params]")
    print("cmd arguments:")
    print("options:")
    print("    -help        : display help")
    print("    -<task> -help: display task help")
    print("task:")
    print("    -lua_bind    : lua bind (use -lua_bind -help to get more infos)")

def main():
    start_time = time.time()
    if len(sys.argv) <= 1:
        print_help()
        exit(1)

    #######################################
    # option
    option_index = 0
    if sys.argv[1] == "-help":
        print_help()
        option_index += 1
    
    if (len(sys.argv) <= option_index + 1):
        exit(1)

    #######################################
    # task
    tasks = collections.OrderedDict()
    tasks["-lua_bind"] = { "run":task_run_lua_bind }

    for i in range(1, len(sys.argv)):
        key = sys.argv[i]
        if key not in tasks.keys():
            continue
        tasks.get(key, )["run"](i)


if __name__ == '__main__':
    main()

