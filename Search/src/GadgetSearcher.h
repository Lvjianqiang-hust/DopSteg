#pragma once
#include "BackTrace.h"
#include "LocationInfo.h"
#include "Interpreter.h"
namespace llvm{
    
    class GadgetSearcher:public ModulePass{
    public:
        using FuncWithType=std::pair<Function*,BackTrace::FuncType>;
        static char ID;
        GadgetSearcher():ModulePass(ID){}
        bool runOnModule(Module &M) override;
        void printGadgetInfo(Module &M);
        void printLocationInfo(Module &M);
    private:
        std::set<Value*> visited;               //搜索过的function
        std::string vuln_func;          //存放漏洞函数
        std::vector<GadgetInfo> gadgets;        //存放gadget信息
        LocationInfo location_info; //漏洞相关信息
    };
}