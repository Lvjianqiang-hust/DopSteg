#pragma once
#include "GadgetInfo.h"

namespace llvm{

    class BackTrace:public InstVisitor<BackTrace>{
    public:
        //用于标记当前操作数是最开始的store指令的“源操作数/目的操作数”
        enum class OperandType{SRC,DST,NONE};
        //函数类型：{发生溢出的函数，其它函数，发生溢出的函数的子函数}
        enum class FuncType{OVERFLOWFUNC,OTHER,CHILD};    
        //{指令/变量，相对应的OperandType}           
        using value_with_type=std::pair<Value*,OperandType>;
        //backtrace经过的指令越多，gadget将会越复杂，越难使用，所以设置了一个上限
        static const int BACKSRCNUM = 1000;
        //源操作数是interesting的store指令集，这里的store指令可能不是gadget，但对后面的gadget分析有帮助
        static std::map<Instruction*,Gadget> interesting_storeinst;
        //（store指令/敏感函数的调用，函数类型，第一个参数是否是敏感函数），构造函数里面直接调用了分析函数
        BackTrace(Instruction* I,FuncType t,bool isc=false);
        // 如果源/目的操作数不全是字符串常量，则返回this->isavailable，否则认为不是gadget
        // backtrace的过程以及这个函数内都是对gadget最直观最简单的初步筛选。
        bool isInteresting();
        //返回gadget本身
        Gadget getGadget(){return this->gadget;}            
        //当遇到没有重写函数的指令时，执行这个
        void visitUnaryInstruction(UnaryInstruction &I);    
        //正常情况下不应该执行
        void visitInstruction(Instruction &I);

        //调用visit(I)后根据指令类型执行下面这些函数              
        void visitLoadInst(LoadInst &I);
        void visitStoreInst(StoreInst &I);
        void visitAllocaInst(AllocaInst &I);
        void visitGetElementPtrInst(GetElementPtrInst &I);
        void visitSelectInst(SelectInst &I);
        void visitPHINode(PHINode &I);
        void visitBitCastInst(BitCastInst &I);
        void visitTruncInst(TruncInst &I);
        void visitSExtInst(SExtInst &I);
        void visitPtrToIntInst(PtrToIntInst &I);
        void visitFPToSIInst(FPToSIInst &I);
        void visitBinaryOperator(BinaryOperator &I);
        void visitInsertElementInst(InsertElementInst &I);
        void visitICmpInst(ICmpInst &I);
        void visitCallInst(CallInst &I);
        
        /*
        void visitCmpInst(CmpInst &I);
        void visitBranchInst(BranchInst &I);
        void visitFCmpInst(FCmpInst &I);
        void visitExtractElementInst(ExtractElementInst &I);
        void visitShuffleVectorInst(ShuffleVectorInst &I);           
        void visitIndirectBrInst(IndirectBrInst &I);*/

    private:
        //backtrace给定的store指令/敏感函数，在构造函数中调用
        void analyzeInstruction();  
        //gadget是否可用
        bool isavailable;                           
        //是否敏感函数调用
        bool iscall;
        //是否经过了复杂的超出预期的函数    
        bool has_a_call;
        //是否遇到了算数运算
        bool is_arithmetic; 
        //是否遇到了特殊的敏感函数调用，如vsnprintf
        std::string special_func; 
        //记录需要插入恢复指令集的位置
        Instruction* pre_inst;
        //gadget本身，里面记录了相关信息  
        Gadget gadget;
        //记录源/目的操作数是不是interesting，为了方便特殊函数的分析，溯源到参数时也判断是interesting  
        std::vector<bool> interesting_flag;
        //记录源/目的操作数是不是interesting，只有针对一些特殊的函数时才认为参数是insteresting的
        //只有interesting_flag和isavail_flag都满足了才可能gadget
        std::vector<bool> isavail_flag;
        //用于分析的容器，其中type表示当前的value来自于最初的store指令的源/目的操作数              
        std::stack<value_with_type> wait2visit;     
        //当前指令属于最初的store指令的源操作数/目的操作数
        OperandType op_type;
        //函数类型                        
        FuncType func_type; 
    };
}