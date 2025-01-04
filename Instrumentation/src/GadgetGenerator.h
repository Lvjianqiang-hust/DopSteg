#pragma once
#include "LLVM.h"
#include <string>

namespace llvm {
class GadgetGenerator {
private:
  IRBuilder<> *builder;
  LLVMContext &context;
  Function *F;
  Module *M;
  Value *a;
  Value *b;
  Value *c;
  Value *d;
  SmallVector<GlobalValue *, 5> global_vars;
  SmallVector<Value *, 5> local_vars;

public:
  GadgetGenerator(LLVMContext &context, Module *M) : context(context), M(M) {
    builder = new IRBuilder<>(context);
  }
  GadgetGenerator(IRBuilder<> *builder, LLVMContext &context, Module *M)
      : builder(builder), context(context), M(M) {}
  GadgetGenerator(LLVMContext &context, Module *M, Function *F)
      :  context(context), M(M), F(F) {}
  void setInsertPoint(Instruction *inst);
  void setInsertPoint(BasicBlock *bb);
  void insertGlobalVariables(StringRef name, StringRef typestr);
  void insertLocalVariables(StringRef name, StringRef typestr);
  void insertAssignment(StringRef src, StringRef srcpos, StringRef dst,
                        StringRef dstpos, StringRef type);
  Value *insertArithmetic(StringRef lhs, StringRef lhspos, StringRef rhs,
                          StringRef rhspos, StringRef dst, StringRef dstpos,
                          StringRef type);
  Value *getVariableValuePointer(StringRef variablestr, StringRef variablepos,
                                 bool is_left_value = false);
  Instruction *getLocalVariablePointer(StringRef variablestr);
  Instruction *insertIfThenBlock(Value *cond, Instruction *split_before);
  Value *insertCall(StringRef name, StringRef param1, StringRef param1pos,
                    StringRef param2, StringRef param2pos, StringRef param3,
                    StringRef param3pos);
  void insertLoopGadgets();
};
} // namespace llvm