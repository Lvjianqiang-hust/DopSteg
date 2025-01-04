#pragma once
#include "LLVM.h"
#include "Util.h"

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace llvm {

class GadgetAttacher {
private:
  Module *M;
  Function *F;
  LoopInfo *LI;

public:
  GadgetAttacher(Module *M) : M(M) {}
  GadgetAttacher(Module *M, Function *F) : M(M), F(F) {}
  GadgetAttacher(Module *M, Function *F, LoopInfo *LI) : M(M), F(F), LI(LI) {}

  void run();
  void attachGlobalVariable(GlobalVariableGadget *global_variable_gadget);
  void attachLocalVariable(LocalVariableGadget *local_variable_gadget);
  void attachAssignment(AssignmentGadget *assignment_gadget);
  void attachArithmetic(ArithmeticGadget *Arithmetic_gadget);
  void attachIfThen(IfThenGadget *ifthen_gadget);
  void attachCall(CallGadget *call_gadget);
  Instruction *getInsertPosition(InsertPoint *point);
  bool attachGadget();
  bool attachGadget(StringRef target);
};
} // namespace llvm
