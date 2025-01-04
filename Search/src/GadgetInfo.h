#pragma once
#include "Utils.h"

namespace llvm{
    class Gadget{
    public:
        Instruction* inst;                 //存储指令本身
        std::vector<Instruction*> src_inst; //追溯源操作数遇到的指令
        std::vector<Instruction*> dst_inst; //追溯目的操作数遇到的指令
        std::map<Value*,Instruction*> src;            //与源操作数有关的参数或全局变量，以及对应的要插入指令的位置
        std::map<Value*,Instruction*> dst;            //与目的操作数有关的参数或全局变量，以及对应的要插入指令的位置
        bool isindex{false};
        int use_num{2};
        Gadget(Instruction* s=nullptr):inst(s){}
        Gadget(Instruction* si,const std::vector<Instruction*>& src,const std::vector<Instruction*>& dst,const std::map<Value*,Instruction*>& s,const std::map<Value*,Instruction*>& d):
        inst(si),src_inst(src),dst_inst(dst),src(s),dst(d){}
    };

    class GadgetInfo{
    public:
        /*
            ASSIGNMENT: (0)a=b
            OUT:    send/write/printf...
            DEREF:  globalx=*globaly
            RWA:    *localp=*localq（这个类型加上一个合适的赋值语句，也可以是实现DEREF的功能）
            ADD:    (0)a=a+b,(1)a=a+k,其中k是常量
            MUL:    (0)a=a*b,(1)a=a*k,其中k是常量
            REM:    (0)a=a%b,(1)a=a&k,其中k是常量
            ARITHMETIC:     其它比较复杂的算数运算
        */
        enum GadgetType{ASSIGNMENT,OUT,DEREF,RWA,ARITHMETIC,XOR,ADD,SUB,MUL,REM,NONE};
        static std::map<Value*,std::pair<bool,int>> val2info;
        static std::map<std::pair<Value*,std::string>,std::pair<bool,int>> struct2info;
        GadgetInfo(){}
        GadgetInfo(const Gadget& g,Function* f,int id,bool iscall);

        friend raw_ostream& operator << (raw_ostream& out,GadgetInfo& g);
        bool isGadget(){return this->isgadget;}
        const Gadget& getGadget(){return this->gadget;}
        GadgetType getType(){return this->gadget_type;}
        Function* getFunc(){return this->func;}
        std::string getGEPIKey(GetElementPtrInst* GEPI);
        std::string getGEPIKey(ConstantExpr* GEPI);

        static void setNeededGadget(const std::vector<std::string>& needed_type){
            needed_gadget.clear();
            for(auto& x:needed_type){
                needed_gadget.insert(s_to_gtype[x]);
            }
        }

        static std::unordered_map<std::string,GadgetType> s_to_gtype;
        static std::unordered_set<GadgetType> needed_gadget;
        
    private:
        void analyzeGadget(bool iscall);                              //分析和筛选gadgets

        bool isgadget;  //通过指令的分析，是否是gadget
        Gadget gadget;  //gadget信息，从BackTrace中拷贝来的，可以用智能指针和move来优化
        GadgetType gadget_type; //gadget类型
        Function* func; //gadget所在函数
        int index; //gadget在函数中的偏移，单位是指令
    };

}

