import re
import json
import sys

def in_quotes(string):
    return "\"" + string + "\""

def src_line(line):
    line += "\n"
    return line

def format_source(source, indent_size):
    formatted = ""
    lines = source.splitlines()
    indent = 0
    indents = ["{"]
    unindnets = ["}"]
    newline = False
    for line in lines:
        if newline and len(line) > 0 and line[0] != "}":
            formatted += "\n"
        newline = False
        cur_indent = indent
        line = line.strip()
        attr = line.find("[[")
        if len(line) < 1 or attr != -1:
            formatted += "\n" ## keep space line
            continue
        for c in line:
            if c in indents:
                indent += 1
            elif c in unindnets:
                indent -= 1
                cur_indent = indent
                newline = True
        formatted += " " * cur_indent * indent_size
        formatted += line
        formatted += "\n"
    return formatted


def test():
    print("code_gen_utils v0.1.0")

if __name__ == "__main__":
    test()