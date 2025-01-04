#pragma once
#include "LLVM.h"

namespace llvm {

class LegacyGlueLayer : public ModulePass {
public:
  static char ID;
  LegacyGlueLayer() : ModulePass(ID) {}
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnModule(Module &M) override;
};
} // namespace llvm
