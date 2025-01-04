#include "GadgetAttacher.h"
#include "GadgetGenerator.h"

using namespace llvm;

void GadgetAttacher::run() { this->attachGadget("bof"); }

void GadgetAttacher::attachGlobalVariable(
    GlobalVariableGadget *global_variable_gadget) {
  GadgetGenerator gadgetgenerator =
      GadgetGenerator(this->M->getContext(), this->M);
  // gadgetgenerator.setInsertPoint(&this->F->getEntryBlock());
  gadgetgenerator.insertGlobalVariables(global_variable_gadget->name,
                                        global_variable_gadget->type);
}
void GadgetAttacher::attachLocalVariable(
    LocalVariableGadget *local_variable_gadget) {
  GadgetGenerator gadgetgenerator =
      GadgetGenerator(this->M->getContext(), this->M);
  // 定位position
  auto position = local_variable_gadget->insertpoint.getInsertPoint();
  assert(position && "insert point must exist");
  gadgetgenerator.setInsertPoint(position);
  // gadgetgenerator.setInsertPoint(
  //     this->F->getEntryBlock().getFirstNonPHIOrDbgOrLifetime());
  gadgetgenerator.insertLocalVariables(local_variable_gadget->name,
                                       local_variable_gadget->type);
}
void GadgetAttacher::attachAssignment(AssignmentGadget *assignment_gadget) {
  GadgetGenerator gadgetgenerator =
      GadgetGenerator(this->M->getContext(), this->M);
  // 定位position
  auto position = assignment_gadget->insertpoint.getInsertPoint();
  assert(position && "insert point must exist");
  gadgetgenerator.setInsertPoint(position);
  gadgetgenerator.insertAssignment(
      assignment_gadget->src, assignment_gadget->srcpos, assignment_gadget->dst,
      assignment_gadget->dstpos, assignment_gadget->type);
}
void GadgetAttacher::attachArithmetic(ArithmeticGadget *arithmetic_gadget) {
  GadgetGenerator gadgetgenerator =
      GadgetGenerator(this->M->getContext(), this->M);
  // 定位position
  auto position = arithmetic_gadget->insertpoint.getInsertPoint();
  assert(position && "insert point must exist");
  gadgetgenerator.setInsertPoint(position);
  gadgetgenerator.insertArithmetic(
      arithmetic_gadget->lhs, arithmetic_gadget->lhspos, arithmetic_gadget->rhs,
      arithmetic_gadget->rhspos, arithmetic_gadget->dst,
      arithmetic_gadget->dstpos, arithmetic_gadget->type);
}
void GadgetAttacher::attachIfThen(IfThenGadget *ifthen_gadget) {
  GadgetGenerator gadgetgenerator =
      GadgetGenerator(this->M->getContext(), this->M, this->F);

  auto position = ifthen_gadget->insertpoint.getInsertPoint();
  assert(position && "insert point must exist");
  gadgetgenerator.setInsertPoint(position);
  // 1. 插入条件icmp指令
  auto cond_gadget = &ifthen_gadget->cond;
  Value *cond = gadgetgenerator.insertArithmetic(
      cond_gadget->lhs, cond_gadget->lhspos, cond_gadget->rhs,
      cond_gadget->rhspos, cond_gadget->dst, cond_gadget->dstpos,
      cond_gadget->type);
  // 2. 插入ifthen分支
  // 因为已经插入一条icmp 根据规则需要跳两条指令
  Instruction *targ = gadgetgenerator.insertIfThenBlock(cond, position);
  // 3. 在then分支插入thenlist多个指令
  gadgetgenerator.setInsertPoint(targ);
  auto then_gadgetlist = ifthen_gadget->thenlist;
  for (auto item : then_gadgetlist) {
    switch (item->kind) {
    case Kind::Assignment: {
      auto assignment_item = static_cast<AssignmentGadget *>(item);
      gadgetgenerator.insertAssignment(
          assignment_item->src, assignment_item->srcpos, assignment_item->dst,
          assignment_item->dstpos, assignment_item->type);
      break;
    }
    case Kind::Arithmetic: {
      auto arithmetic_item = static_cast<ArithmeticGadget *>(item);
      gadgetgenerator.insertArithmetic(
          arithmetic_item->lhs, arithmetic_item->lhspos, arithmetic_item->rhs,
          arithmetic_item->rhspos, arithmetic_item->dst,
          arithmetic_item->dstpos, arithmetic_item->type);
      break;
    }
    case Kind::Call: {
      auto call_item = static_cast<CallGadget *>(item);
      gadgetgenerator.insertCall(
          call_item->name, call_item->parameter1, call_item->parameter1Pos,
          call_item->parameter2, call_item->parameter2Pos,
          call_item->parameter3, call_item->parameter3Pos);
      break;
    }

    default:
      assert(false && "only support arithmetic gadget in then block");
      break;
    }
  }
}
void GadgetAttacher::attachCall(CallGadget *call_gadget) {
  GadgetGenerator gadgetgenerator =
      GadgetGenerator(this->M->getContext(), this->M);
  // 定位position
  auto position = call_gadget->insertpoint.getInsertPoint();
  assert(position && "insert point must exist");
  gadgetgenerator.setInsertPoint(position);
  gadgetgenerator.insertCall(
      call_gadget->name, call_gadget->parameter1, call_gadget->parameter1Pos,
      call_gadget->parameter2, call_gadget->parameter2Pos,
      call_gadget->parameter3, call_gadget->parameter3Pos);
}
Instruction *GadgetAttacher::getInsertPosition(InsertPoint *point) {
  for (auto &bb : *this->F) {
    for (auto &inst : bb) {
      // inst.print(errs());
      if (point->instkind == "call") {
        if (CallInst *call_inst = dyn_cast<CallInst>(&inst)) {
          if (call_inst->getCalledFunction()->getName().startswith(
                  point->nickname)) {
            std::string helper;
            raw_string_ostream tmp(helper);
            call_inst->print(tmp);
            if (helper.find(point->helper) != std::string::npos) {
              if (point->direction == "after") {
                return inst.getNextNode();
              } else {
                return &inst;
              }
            }
          }
        }
      } else if (point->instkind == "store") {
        if (StoreInst *store_inst = dyn_cast<StoreInst>(&inst)) {
          if (store_inst->getPointerOperand()->getName().startswith(
                  point->nickname)) {
            std::string helper;
            raw_string_ostream tmp(helper);
            store_inst->print(tmp);
            if (helper.find(point->helper) != std::string::npos) {
              if (point->direction == "after") {
                return inst.getNextNode();
              } else {
                return &inst;
              }
            }
          }
        }
      }
    }
  }
  return nullptr;
}

// 通过模板装饰Generator的方法扩充 但是好像并不需要 直接继承就行
// 也可使用工厂模式 创建不通类型的Generator

bool GadgetAttacher::attachGadget() {
  // generate gadgets for a specific "function"
  // auto &context = this->F->getContext();
  // // IRBuilder<> builder(context);
  // // GadgetGenerator gadgetgenerator = GadgetGenerator(&builder, context);
  // GadgetGenerator gadgetgenerator = GadgetGenerator(context, F->getParent());
  // gadgetgenerator.setTarget("strcpy");
  // auto *m = this->F->getParent();
  // bool find = false;
  // for (auto &bb : *this->F) {
  //   if (find)
  //     break;
  //   for (auto &inst : bb) {
  //     if (find)
  //       break;
  //     if (CallInst *call_inst = dyn_cast<CallInst>(&inst)) {
  //       Function *called_func = call_inst->getCalledFunction();
  //       if (called_func->getName() == gadgetgenerator.getTarget()) {
  //         find = true;
  //         // print log
  //         errs() << "Find target function: " << called_func->getName() <<
  //         "\n";
  //         // builder.SetInsertPoint(call_inst->getNextNode());
  //         auto &entry_block = this->F->getEntryBlock();
  //         auto test_inst = entry_block.getFirstNonPHIOrDbgOrLifetime();
  //         if (AllocaInst *tmpinst = dyn_cast<AllocaInst>(test_inst)) {
  //           errs() << test_inst->getName();
  //           errs() << tmpinst->getName();
  //           errs() << tmpinst->getValueName();
  //         }
  //         gadgetgenerator.setInsertPoint(
  //             entry_block.getFirstNonPHIOrDbgOrLifetime());
  //         gadgetgenerator.insertVariables();
  //       }
  //     }
  //   }
  // }
  // if (LI)
  //   for (auto L : *LI) {
  //     auto entry_block = *(L->block_begin());
  //     gadgetgenerator.setInsertPoint(
  //         entry_block->getFirstNonPHIOrDbgOrLifetime());
  //     gadgetgenerator.insertFunctionalGadgets();
  //     break;
  //     L->getExitBlock();
  //   }
  // // loop
  // auto tmp = this->F->getBasicBlockList().rbegin();
  // gadgetgenerator.setInsertPoint(&*tmp);
  // gadgetgenerator.insertLoopGadgets();
  return true;
}

bool GadgetAttacher::attachGadget(StringRef target) {
  // generate gadgets for a specific "function"
  // auto &context = this->F->getContext();
  // if (F->getName() != target) {
  //   return false;
  // }
  // GadgetGenerator gadgetgenerator = GadgetGenerator(context, F->getParent());
  // auto *m = this->F->getParent();
  // gadgetgenerator.setInsertPoint(&this->F->getEntryBlock());
  // gadgetgenerator.insertGlobalVariables();
  // bool find = false;
  // for (auto &bb : *this->F) {
  //   if (find)
  //     break;
  //   for (auto &inst : bb) {
  //     if (find)
  //       break;
  //     if (CallInst *call_inst = dyn_cast<CallInst>(&inst)) {
  //       Function *called_func = call_inst->getCalledFunction();
  //       if (called_func->getName().find("memcpy") != StringRef::npos) {
  //         find = true;
  //         // print log
  //         errs() << "Find target function: " << called_func->getName() <<
  //         "\n";
  //         // builder.SetInsertPoint(call_inst->getNextNode());
  //         gadgetgenerator.setInsertPoint(
  //             this->F->getEntryBlock().getFirstNonPHIOrDbgOrLifetime());
  //         gadgetgenerator.insertLocalVariables();
  //         gadgetgenerator.setInsertPoint(inst.getNextNode());
  //         gadgetgenerator.insertLoad("key");
  //         gadgetgenerator.insertStore("buf");
  //         gadgetgenerator.insertIfCond();
  //         // gadgetgenerator.insertCalc();
  //       }
  //     }
  //   }
  // }
  return true;
}
