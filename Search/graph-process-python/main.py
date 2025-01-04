import graphviz
import sys


def reverse_map(raw_mp):
    reverse_mp = {}
    for key, value in raw_mp.items():
        for v in value:
            if v in reverse_mp:
                reverse_mp[v].append(key)
            else:
                reverse_mp[v] = [key]
    return reverse_mp


def exclude_path(graph_mp, exclude_node):
    for key, value in graph_mp.items():
        if key in exclude_node:
            graph_mp[key] = []
        else:
            new_value = []
            for v in value:
                if v not in exclude_node:
                    new_value.append(v)
            graph_mp[key] = new_value
    return graph_mp


def depth_limit(graph_mp, start: str, end, depth):
    # 根据 depth 限制图遍历从start到end的深度
    visited = set()
    queue = [(start, int(0))]
    while queue:
        node, d = queue.pop(0)
        if node in visited:
            continue
        visited.add(node)
        if d >= depth:
            continue
        if node in graph_mp:
            for n in graph_mp[node]:
                queue.append((n, d + 1))
    new_graph_mp = {}
    for key, value in graph_mp.items():
        if key in visited:
            new_value = []
            for v in value:
                if v in visited:
                    new_value.append(v)
            new_graph_mp[key] = new_value
    return new_graph_mp


def reverse_build_graph(raw_mp, start, end):
    # 逆向建图，从end到start
    reverse_graph_mp = {}
    visited = set()
    queue = [end]
    while queue:
        node = queue.pop(0)
        if node in visited:
            continue
        visited.add(node)
        if node in raw_mp:
            for n in raw_mp[node]:
                if node in reverse_graph_mp:
                    reverse_graph_mp[node].append(n)
                else:
                    reverse_graph_mp[node] = [n]
                queue.append(n)
    return reverse_graph_mp


def build_graph(raw_mp, start, end):
    graph_mp = {}
    visited = set()
    queue = [start]
    while queue:
        node = queue.pop(0)
        if node in visited:
            continue
        visited.add(node)
        if node in raw_mp:
            for n in raw_mp[node]:
                if node in graph_mp:
                    graph_mp[node].append(n)
                else:
                    graph_mp[node] = [n]
                queue.append(n)
    return graph_mp


def calc_path(graph_mp, start, end, function_contribution_set, depth_limit=6):
    # 计算路径权值
    # 全部遍历
    # dfs
    path_set = set()
    stack = [(start, int(0), [start])]
    while stack:
        node, depth, path = stack.pop()
        if depth >= depth_limit:
            continue
        if node == end:
            path_set.add(tuple(path))
            continue
        if node in graph_mp:
            for n in graph_mp[node]:
                # if n in function_contribution_set:
                stack.append((n, depth+1, path + [n]))
    function_contribution_set.add(end)
    function_contribution_set.add(start)
    res_set = set()
    for path in path_set:
        cnt = 0
        for node in path:
            if node in function_contribution_set:
                cnt += 1
        res_set.add((path, cnt/len(path)))
    sorted_res = sorted(res_set, key=lambda x: x[1], reverse=True)
    return sorted_res


if __name__ == "__main__":
    file_path = "proftpd.ll.pta"
    function_contribution_path = "proftpd_INFO"
    # source = "cmd_loop"
    # destination = "sreplace"
    source = sys.argv[1]
    destination = sys.argv[2]
    depth = 3
    exclude_node = {"pr_fsio_close", "pr_fsio_closedir"}
    raw_mp = {}
    with open(file_path, "r") as f:
        while True:
            line = f.readline()
            if not line:
                break
            line = line.strip()
            words = line.split()
            raw_mp[words[0]] = words[1:]

    # 再建逆向图
    reverse_mp = reverse_map(raw_mp)
    # 读取函数贡献
    function_contribution_set = set()
    with open(function_contribution_path, "r") as f:
        while True:
            line = f.readline()
            if not line:
                break
            line = line.strip()
            for word in line.split():
                function_contribution_set.add(word)
    # 先逆向建图，筛掉不可达的点
    reverse_graph_mp = reverse_build_graph(reverse_mp, source, destination)
    # 限制深度
    reverse_graph_mp = depth_limit(
        reverse_graph_mp, destination, source, depth)
    reverse_graph_mp = reverse_map(reverse_graph_mp)
    # 再正向建图，筛掉不可达的点
    graph_mp = build_graph(reverse_graph_mp, source, destination)
    # 限制深度
    graph_mp = depth_limit(graph_mp, source, destination, depth)
    graph_mp = exclude_path(graph_mp, exclude_node)

    # 计算权值并输出路径
    path_set = calc_path(graph_mp, source, destination,
                         function_contribution_set)
    with open("path.txt", "w") as f:
        for path, weight in path_set:
            f.write(str(path) + " " + str(weight) + "\n")

    dot = graphviz.Digraph(comment='The Round Table')
    for key, value in graph_mp.items():
        dot.node(key, key)
        for v in value:
            dot.edge(key, v)
    # print(dot.source)
    dot.format = 'png'
    dot.render('path.gv')
