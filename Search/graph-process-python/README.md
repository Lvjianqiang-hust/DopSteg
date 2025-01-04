# requirements

# 路径探索

基于指针分析赋能的完整CFG（Control Flow Graph）函数调用图，进行潜在利用的DOP路径探索并展示。

```bash
sudo apt-get install python3-graphviz
```
# usage

```bash
# run the path analysis from "cmd_loop" to "sreplace"
python3 main.py cmd_loop sreplace
# output: path.txt and path.gv.png
```