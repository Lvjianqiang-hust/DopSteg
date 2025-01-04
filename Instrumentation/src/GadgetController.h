#pragma once
#include "LLVM.h"
#include "Util.h"

#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace llvm {

class GadgetController {
private:
  Module *M;
  LoopInfoWrapperPass *LIWP;

public:
  GadgetController(Module *M) : M(M) {}
  GadgetController(Module *M, LoopInfoWrapperPass *LIWP) : M(M), LIWP(LIWP) {}
  std::vector<GadgetToInsert *> configParser(StringRef config_path);
  Kind getKind(string kindstr);
  Instruction *getInsertPoint(GadgetToInsert *item);
  void run();
};
} // namespace llvm
