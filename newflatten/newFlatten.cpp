#include <llvm-10/llvm/IR/BasicBlock.h>
#include <llvm-10/llvm/IR/Instructions.h>
#include <llvm-10/llvm/IR/Value.h>
#include <llvm-10/llvm/Support/Casting.h>
#include <sys/types.h>

#include <cstdint>
#include <map>
#include <vector>

#include "llvm/ADT/ilist.h"
#include "llvm/IR/Function.h"  // Function相关
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"  // PassManager相关
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"  // 用于输出到终端
#include "llvm/Support/CommandLine.h"
#include "map"
#include <string>

using namespace llvm;

static cl::opt<std::string> flattenFunction("sos-name", cl::desc("a function name to flatten"), cl::Required);

// 定义一个自定义 Function Pass
class allNamePass : public PassInfoMixin<allNamePass> {
   public:
    // `run` 方法是 Pass 的入口
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
        /* if (F.getName() == "_Z7processP4Node") { */
        if (F.getName() == flattenFunction) {
            errs() << "flatten is here!\n";
            splice(F);
            flatten(F);
        }
        return PreservedAnalyses::none();
    }

   private:
    bool containWhite(uint32_t number) {
        int byte = 0x000000ff & number;
        bool flag = false;
        if((byte <= 13 && byte >= 9) || byte == 32 || !byte) flag = true;
        byte = 0x000000ff & (number >> 8);
        if((byte <= 13 && byte >= 9) || byte == 32 || !byte) flag = true;
        byte = 0x000000ff & (number >> 16);
        if((byte <= 13 && byte >= 9) || byte == 32 || !byte) flag = true;
        byte = 0x000000ff & (number >> 24);
        if((byte <= 13 && byte >= 9) || byte == 32 || !byte) flag = true;
        return flag;
    }
    uint32_t getNextNumber(uint32_t &number) {
        number++;
        while (containWhite(number)) {
            number++;
        }
        return number;
    }
    void flatten(Function &F) {
        // init
        LLVMContext &Ctx = F.getContext();
        IRBuilder<> Builder(Ctx);

        BasicBlock *entry = &F.getEntryBlock();
        // create init bb and dispatcherBB
        BasicBlock *initBB = BasicBlock::Create(Ctx, "initBB", &F, entry);
        BasicBlock *dispatcherBB = BasicBlock::Create(Ctx, "dispatcherBB", &F);

        // create bb to int map
        std::map<BasicBlock *, uint32_t> block2Int;
        uint32_t number = 0;
        for (auto &BB : F) {
            block2Int[&BB] = getNextNumber(number);
        }

        // write in init bb
        Builder.SetInsertPoint(initBB);
        AllocaInst *dispatcherFlag =
            Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "dispatcherFlag");
        AllocaInst *attackMe =
            Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "attackMe");
        AllocaInst *state =
            Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "state");
        Builder.CreateStore(Builder.getInt32(block2Int[entry]), state);
        Builder.CreateStore(Builder.getInt32(0), dispatcherFlag);
        Builder.CreateBr(dispatcherBB);

        std::vector<AllocaInst *>dispatcherVector = {dispatcherFlag, attackMe, state};

        // write in dispatcher bb
        Builder.SetInsertPoint(dispatcherBB);
        Value *loadState = Builder.CreateLoad(state, "loadState");
        SwitchInst *switchInst = Builder.CreateSwitch(loadState, dispatcherBB);

        // move AllocaInst to the entry
        std::vector<Instruction *> toMove;
        for (auto &BB : F) {
            if (BB.getName() == "dispatcherBB" || BB.getName() == "initBB")
                continue;
            for (auto &I : BB) {
                if (auto *AI = dyn_cast<AllocaInst>(&I)) {
                    toMove.push_back(&I);
                }
            }
        }
        for (Instruction *I : toMove) {
            I->moveBefore(initBB->getTerminator());
        }

        for (auto &BB : F) {
            if (BB.getName() == "dispatcherBB" || BB.getName() == "initBB")
                continue;
            auto tmp = block2Int.find(&BB);
            if (tmp == block2Int.end()) continue;
            // complement switch case
            switchInst->addCase(Builder.getInt32(tmp->second), &BB);
            // find next block
            handleTerminator(BB, Builder, block2Int, dispatcherBB, dispatcherVector);
        }
    }
    void handleTerminator(BasicBlock &BB, IRBuilder<> &Builder,
                          std::map<BasicBlock *, uint32_t> &block2Int,
                          BasicBlock *dispatcherBB, const std::vector<AllocaInst *>dispatcherVector) {
        Instruction *terminator = BB.getTerminator();
        if (auto *BI = dyn_cast<BranchInst>(terminator)) {
            // br command
            if (BI->isUnconditional()) {
                // uncondition br
                BasicBlock *nextBB = BI->getSuccessor(0);
                Builder.SetInsertPoint(&BB.front());
                Value *flag = Builder.CreateLoad(Builder.getInt32Ty(), dispatcherVector[0]);
                Value *isAttack = Builder.CreateICmpNE(flag, Builder.getInt32(0));
                Value *attackMe = Builder.CreateLoad(Builder.getInt32Ty(), dispatcherVector[1]);
                Value *NoNextBB = Builder.CreateSelect(isAttack, attackMe, Builder.getInt32(block2Int[nextBB]));
                Builder.CreateStore(NoNextBB, dispatcherVector[2]);

                terminator->eraseFromParent();
                Builder.SetInsertPoint(&BB);
                Builder.CreateBr(dispatcherBB);
            } else if (BI->isConditional()) {
                // condition br
                // get conditionVar and nextBB
                Value *conditionVal = BI->getCondition();
                BasicBlock *nextTrueBB = BI->getSuccessor(0);
                BasicBlock *nextFalseBB = BI->getSuccessor(1);

                // use select to get next BB
                Builder.SetInsertPoint(terminator);
                Value *selectVal = Builder.CreateSelect(
                    conditionVal, Builder.getInt32(block2Int[nextTrueBB]),
                    Builder.getInt32(block2Int[nextFalseBB]), "selectVal");
                Value *flag = Builder.CreateLoad(Builder.getInt32Ty(), dispatcherVector[0]);
                Value *isAttack = Builder.CreateICmpNE(flag, Builder.getInt32(0));
                Value *attackMe = Builder.CreateLoad(Builder.getInt32Ty(), dispatcherVector[1]);
                Value *NoNextBB = Builder.CreateSelect(isAttack, attackMe, selectVal);
                Builder.CreateStore(NoNextBB, dispatcherVector[2]);

                terminator->eraseFromParent();
                Builder.SetInsertPoint(&BB);
                Builder.CreateBr(dispatcherBB);
            }
        } else if (auto *RI = dyn_cast<ReturnInst>(terminator)) {
            // ret inst no need to change
        } else if (auto *SI = dyn_cast<SwitchInst>(terminator)) {
            errs() << "no support for switch";
        } else {
            errs() << "unknow IR";
        }
    }
    void splice(Function &F) {
        std::vector<BasicBlock *> oldBBs;
        for (auto &BB : F) {
            oldBBs.push_back(&BB);
        }
        for (auto BB : oldBBs) {
            BasicBlock *firstBB = spliceBlock(*BB);
            BB->replaceAllUsesWith(firstBB);
            BB->eraseFromParent();
        }
    }
    BasicBlock *spliceBlock(BasicBlock &BB) {
        // todo check the BasicBlock Value
        Function *F = BB.getParent();
        LLVMContext &Ctx = F->getContext();
        IRBuilder<> Builder(Ctx);
        std::vector<Instruction *> Instruction2Splice;
        for (auto &I : BB) {
            if (dyn_cast<StoreInst>(&I)) {
                Instruction2Splice.push_back(&I);
            } else if (dyn_cast<CallInst>(&I) && dyn_cast<CallInst>(&I)->use_empty()) {
                Instruction2Splice.push_back(&I);
            }
        }
        Instruction2Splice.push_back(BB.getTerminator());
        int spliceNumber = Instruction2Splice.size();
        BasicBlock *lastBB = nullptr;
        BasicBlock *firstBB = nullptr;
        for (int i = 0; i < spliceNumber; i++) {
            BasicBlock *newBB = BasicBlock::Create(Ctx, "", F, &BB);
            if (i == 0) {
                firstBB = newBB;
            }
            newBB->getInstList().splice(newBB->end(), BB.getInstList(),
                                        BB.begin(),
                                        ++BasicBlock::iterator(Instruction2Splice[i]));
            if (lastBB) {
                Builder.SetInsertPoint(lastBB);
                Builder.CreateBr(newBB);
            }
            lastBB = newBB;
        }
        return firstBB;
    }
    void test(Function &F, FunctionAnalysisManager &FAM) {
        errs() << "nouse is here!\n";
        auto &firstBB = F.front();
        LLVMContext &Ctx = F.getContext();
        IRBuilder<> Builder(Ctx);
        Builder.SetInsertPoint(firstBB.getTerminator());
        AllocaInst *Alloca =
            Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "i32ptr");
        Value *Int32Val = Builder.getInt32(123);
        Builder.CreateStore(Int32Val, Alloca);
        Value *LoadVal = Builder.CreateLoad(Alloca, "loadVar");
        Value *sumVal = Builder.CreateAdd(Int32Val, LoadVal, "sumVal");
        Value *cmpVal = Builder.CreateICmpEQ(Int32Val, sumVal);

        BasicBlock *EQBB = BasicBlock::Create(Ctx, "valEQ", &F);
        BasicBlock *NEBB = BasicBlock::Create(Ctx, "valNE", &F);
        Builder.CreateCondBr(cmpVal, EQBB, NEBB);

        Builder.SetInsertPoint(EQBB);
        Builder.CreateRet(Int32Val);

        // ne code
        Builder.SetInsertPoint(NEBB);
        // create bb for switch
        BasicBlock *case1 = BasicBlock::Create(Ctx, "case1", &F);
        BasicBlock *case2 = BasicBlock::Create(Ctx, "case2", &F);
        BasicBlock *def = BasicBlock::Create(Ctx, "def", &F);
        BasicBlock *end = BasicBlock::Create(Ctx, "end", &F);
        // create stateVal for switch
        AllocaInst *Alloca1 =
            Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "i32ptr1");
        Builder.CreateStore(Int32Val, Alloca1);
        Value *LoadVal1 = Builder.CreateLoad(Alloca1, "loadVar1");
        // create switch code and add case
        SwitchInst *switchInst = Builder.CreateSwitch(LoadVal1, def);
        switchInst->addCase(Builder.getInt32(1), case1);
        switchInst->addCase(Builder.getInt32(2), case2);
        // case1,2 code
        Builder.SetInsertPoint(case1);
        Builder.CreateBr(end);
        Builder.SetInsertPoint(case2);
        Builder.CreateBr(end);
        Builder.SetInsertPoint(def);
        Builder.CreateBr(end);
        Builder.SetInsertPoint(end);
        Builder.CreateRet(Int32Val);
        // Value *const32 = Builder.getInt32(123);
    }
    void getInfo(Function &F, FunctionAnalysisManager &FAM) {
        errs() << "Function : " << F.getName() << "\n";
        errs() << "Ret Type : " << *(F.getReturnType()) << "\n";
        int i = 1;
        for (auto &Arg : F.args()) {
            if (Arg.getName().empty()) Arg.setName("unamedArg");
            errs() << "Arg" << i << " : " << Arg.getName() << ' ';
            Arg.getType()->print(errs());
            errs() << "\n";
            i++;
        }
        for (auto &BB : F) {
            if (BB.getName().empty()) BB.setName("unamedBasicBlock");
            errs() << "  Basic Block : " << BB.getName() << "\n";
            for (auto &I : BB) {
                errs() << "    IR : " << I << "\n";
            }
        }
    }
};

// 插件的注册信息
extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION,  // LLVM 插件 API 版本
            "allNamePass",            // 插件名称
            LLVM_VERSION_STRING,      // LLVM 版本
            [](PassBuilder &PB) {     // 注册 Pass 的回调
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "all-name") {  // Pass 名称
                            FPM.addPass(allNamePass());
                            return true;
                        }
                        return false;
                    });
            }};
}
