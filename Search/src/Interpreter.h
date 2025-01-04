#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include <unordered_set>
#include <unordered_map>

//  Attack Semantic Description Language(ASDL)

class Interpreter{
public:
    enum class LEVEL{MODIFIABLE,NONMODIFIABLE};
    Interpreter(const std::string& file);
    void analyzeASDL(LEVEL level=LEVEL::MODIFIABLE);
    std::vector<std::string> getNeededType(){return this->needed_type;}
    std::vector<std::string> getNeededSYS(){return this->needed_SYS;}
    bool isNeededDispatcher(){return this->is_needed_dispathcer;}
private:
    std::string filename;       //描述攻击语义的文件
    std::vector<std::string> needed_type;
    std::vector<std::string> needed_SYS;
    bool is_needed_dispathcer;
};