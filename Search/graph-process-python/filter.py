import os
import re


def filter_map_set(raw_mp, source):
    # 过滤raw_map，只保留source开始的路径集合
    visited = set()
    queue = [source]
    while queue:
        node = queue.pop(0)
        if node in visited:
            continue
        visited.add(node)
        if node in raw_mp:
            for n in raw_mp[node]:
                queue.append(n)

    return visited


# 读取gadgets文件并提取出以FUNC: 开头的函数
# 保存到function.txt中

def extract_function(gadgets_file, functions_file):
    with open(gadgets_file, 'r') as f:
        gadgets = f.readlines()
    functions = set()
    for gadget in gadgets:
        if gadget.startswith('FUNC:'):
            functions.add(gadget.strip("FUNC:").strip())
    with open(functions_file, 'w') as f:
        for func in functions:
            f.write(func + '\n')


if __name__ == "__main__":
    gadgets_path = "../Result/proftpd/gadgets"
    gadgets_contribution_path = "../Result/proftpd/gadgets_contribution"
    function_contribution_path = "./graph-process-python/proftpd_function.txt"
    function_contribution_set_path = "./graph-process-python/function_contribution_set.txt"
    function_non_contribution_set_path = "./graph-process-python/function_non_contribution_set.txt"
    file_path = "./graph-process-python/proftpd.ll.pta"

    source = "cmd_loop"
    extract_function(gadgets_path, function_contribution_path)
    # 读取路径
    raw_mp = {}
    with open(file_path, "r") as f:
        while True:
            line = f.readline()
            if not line:
                break
            line = line.strip()
            words = line.split()
            raw_mp[words[0]] = words[1:]
    # 读取数据
    function_contribution_set = set()
    with open(function_contribution_path, "r") as f:
        while True:
            line = f.readline()
            if not line:
                break
            line = line.strip()
            for word in line.split():
                function_contribution_set.add(word)
    print("function_contribution_set size: ", len(function_contribution_set))
    # 以source开始过滤raw_map
    filter_set = filter_map_set(raw_mp, source)
    # 根据function_contribution_set筛选不在filter_set中的点
    cnt = 0
    function_non_contribution_set = function_contribution_set - filter_set
    function_contribution_set = function_contribution_set & filter_set
    # 将 function_contribution_set 和 function_non_contribution_set 分别写入文件
    with open(function_contribution_set_path, "w") as f:
        for func in function_contribution_set:
            f.write(func + "\n")
    with open(function_non_contribution_set_path, "w") as f:
        for func in function_non_contribution_set:
            f.write(func + "\n")
    print("function_contribution_set size: ", len(function_contribution_set))
    print("function_non_contribution_set size: ",
          len(function_non_contribution_set))

    # 读取gadgets文件并筛选在function_contribution_set中的段落
    function = ""
    gadget_type = ""
    line_cache = []
    cnt = 1
    with open(gadgets_path, "r") as f, open(gadgets_contribution_path, "w") as ff:
        while True:
            line = f.readline()
            if line == '\n':
                # 判断function是否在function_contribution_set中
                if function in function_contribution_set or gadget_type == "OUT":
                    # 写入文件
                    ff.write(str(cnt) + "\n")
                    for line in line_cache[1:]:
                        ff.write(line)
                    cnt += 1
                    ff.write("\n\n")
                line_cache = []
                function = ""
                gadget_type = ""
                continue
            if not line:
                break
            line_cache.append(line)
            if line.startswith("FUNC:"):
                function = line.strip("FUNC:").strip()
            if line.startswith("TYPE:"):
                gadget_type = line.strip("TYPE:").strip()
    with open(gadgets_path, "w") as f, open(gadgets_contribution_path, "r") as ff:
        while True:
            line = ff.readline()
            if not line:
                break
            f.write(line)
