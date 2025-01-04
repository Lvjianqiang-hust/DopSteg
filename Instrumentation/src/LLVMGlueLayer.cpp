#include "LLVMGlueLayer.h"
#include "GadgetController.h"

using namespace llvm;

void LegacyGlueLayer::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<LoopInfoWrapperPass>();
  // AU.addRequiredTransitive<LoopInfoWrapperPass>();
  //  UA.addRequiredTransitive<DominatorTreeWrapperPass>();
}
char LegacyGlueLayer::ID = 0;
bool LegacyGlueLayer::runOnModule(Module &M) {
  // auto LIWP = &getAnalysis<LoopInfoWrapperPass>();
  // auto LI = &getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
  //  GadgetController contorller(M, LIWP);
  GadgetController contorller(&M);
  contorller.run();
  return true;
}

static RegisterPass<LegacyGlueLayer> X("gadgetattach", "Attach Gadgets");

/* Legacy PM Registration */
/*static llvm::RegisterStandardPasses
    RegisterGlueLayer(llvm::PassManagerBuilder::EP_EnabledOnOptLevel0,
                      [](const llvm::PassManagerBuilder &Builder,
                         llvm::legacy::PassManagerBase &PM) {
                        PM.add(new llvm::LegacyGlueLayer());
                      });*/
