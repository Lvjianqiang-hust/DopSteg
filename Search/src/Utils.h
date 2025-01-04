#pragma once
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/LoopPass.h" 
#include "llvm-c/Core.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "Interpreter.h"

using namespace llvm;

namespace llvm{

    
    std::string getString();

    template<typename T,typename... TL>
    std::string getString(const T& t,const TL&... args){
        std::string empty;
        raw_string_ostream s(empty);
        s<<t;
        return s.str()+getString(args...);
    }

    class ILogger{
    public:
        enum class Level {ERR,DEBUG,NONE};
        virtual ~ILogger() = default;
        virtual void log(const std::string& _str) = 0;
        std::string getLevelStr(Level _level);
    };

    class Logger : public ILogger{
    public:
        Logger(const std::string& _file, Level _level);
        ~Logger();
        void log(const std::string& _str) override;
        void setLevel(Level _level);
    private:
        raw_fd_ostream out;
        std::error_code EC;
        Level level;
    };

    class Global{
    public:
        static std::unique_ptr<ILogger> logger;
        static std::set<std::string> copy_func;
        static std::set<std::string> print_func;
    };
    
}