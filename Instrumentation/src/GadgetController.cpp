#include "GadgetController.h"
#include "GadgetAttacher.h"
#include "GadgetGenerator.h"
#include "Util.h"

using namespace llvm;

static cl::opt<std::string>
    configPath("config-path", cl::init("gadgetattach.json"),
               cl::desc("function to be attached gadgets"));

Instruction *GadgetController::getInsertPoint(GadgetToInsert *item) {
  auto point = item->getPointInfo();
  Function *target = this->M->getFunction(item->function);
  if (!target || !point) {
    return nullptr;
  }
  int cnt = 1;
  if (point->nickname != "null") {
    cnt = std::stoi(point->nickname);
  }
  for (auto &bb : *target) {
    for (auto &inst : bb) {
      std::string helper;
      raw_string_ostream tmp(helper);
      inst.print(tmp);
      if (helper.find(point->helper) != std::string::npos && --cnt == 0) {
        if (point->direction == "after") {
          return inst.getNextNode();
        } else {
          return &inst;
        }
      }
    }
  }
  return nullptr;
}

void GadgetController::run() {
  std::vector<GadgetToInsert *> gadget_list = this->configParser(configPath);
  // 排序 将变量声明前置
  std::sort(
      gadget_list.begin(), gadget_list.end(),
      [](GadgetToInsert *a, GadgetToInsert *b) { return a->kind < b->kind; });

  //填充gadget_list中的insert_point指针
  Instruction *prev_insert_point = nullptr;
  for (auto item : gadget_list) {
    // item 中 Module 应当与当前 module 相同
    if (this->M->getModuleIdentifier().find(item->module) ==
        std::string::npos) {
      continue;
    }
    if (item->getPointInfo() && item->getPointInfo()->helper != "SameAsAbove") {
      prev_insert_point = this->getInsertPoint(item);
    }
    item->setPointInfo(prev_insert_point);
  }

  //根据gadget_list分发
  for (auto item : gadget_list) {
    // item 中 Module 应当与当前 module 相同
    if (this->M->getModuleIdentifier().find(item->module) ==
        std::string::npos) {
      continue;
    }
    switch (item->kind) {
    case Kind::GlobalVariable: {
      GadgetAttacher attacher(this->M);
      auto global_variable_gadget = static_cast<GlobalVariableGadget *>(item);
      attacher.attachGlobalVariable(global_variable_gadget);
      break;
    }
    case Kind::LocalVariable: {
      // 定位module和function
      Function *target = this->M->getFunction(item->function);
      assert(target && "Target function not found!");
      GadgetAttacher attacher(this->M, target);
      auto local_variable_gadget = static_cast<LocalVariableGadget *>(item);
      attacher.attachLocalVariable(local_variable_gadget);
      break;
    }
    case Kind::Assignment: {
      // 定位module和function
      Function *target = this->M->getFunction(item->function);
      assert(target && "Target function not found!");
      GadgetAttacher attacher(this->M, target);
      auto assignment_gadget = static_cast<AssignmentGadget *>(item);
      attacher.attachAssignment(assignment_gadget);
      break;
    }
    case Kind::Arithmetic: {
      // 定位module和function
      Function *target = this->M->getFunction(item->function);
      assert(target && "Target function not found!");
      GadgetAttacher attacher(this->M, target);
      auto arithmetic_gadget = static_cast<ArithmeticGadget *>(item);
      attacher.attachArithmetic(arithmetic_gadget);
      break;
    }
    case Kind::IfThen: {
      Function *target = this->M->getFunction(item->function);
      assert(target && "Target function not found!");
      GadgetAttacher attacher(this->M, target);
      auto ifthen_gadget = static_cast<IfThenGadget *>(item);
      attacher.attachIfThen(ifthen_gadget);
      break;
    }
    case Kind::Call: {
      Function *target = this->M->getFunction(item->function);
      assert(target && "Target function not found!");
      GadgetAttacher attacher(this->M, target);
      auto call_gadget = static_cast<CallGadget *>(item);
      attacher.attachCall(call_gadget);
      break;
    }

    default:
      assert(false && "cannot support other Kind");
      break;
    }
  }
}

std::vector<GadgetToInsert *>
GadgetController::configParser(StringRef config_path) {
  std::vector<GadgetToInsert *> gadget_list;
  try {
    std::ifstream f(config_path);
    json data = json::parse(f);
    if (data["Task"].get<std::string>() == "Stack Overflow") {
      for (auto &item : data["GadgetList"]) {
        Kind kind = this->getKind(item["Kind"].get<std::string>());
        // errs() << (std::string)item["Kind"] << '\n';
        switch (kind) {
        case Kind::GlobalVariable:
          gadget_list.push_back(
              new GlobalVariableGadget(kind, item["Module"], item["Function"],
                                       item["Name"], item["Type"]));
          break;
        case Kind::LocalVariable:
          gadget_list.push_back(new LocalVariableGadget(
              kind, item["Module"], item["Function"], item["Name"],
              item["Type"], item["InsertPosition"]["InstructionKind"],
              item["InsertPosition"]["NickName"],
              item["InsertPosition"]["Helper"],
              item["InsertPosition"]["Direction"]));
          break;
        case Kind::Assignment:
          gadget_list.push_back(new AssignmentGadget(
              kind, item["Module"], item["Function"], item["Src"],
              item["SrcPos"], item["Dst"], item["DstPos"], item["Type"],
              item["InsertPosition"]["InstructionKind"],
              item["InsertPosition"]["NickName"],
              item["InsertPosition"]["Helper"],
              item["InsertPosition"]["Direction"]));
          break;
        case Kind::Arithmetic:
          gadget_list.push_back(new ArithmeticGadget(
              kind, item["Module"], item["Function"], item["LHS"],
              item["LHSPos"], item["RHS"], item["RHSPos"], item["Dst"],
              item["DstPos"], item["Type"],
              item["InsertPosition"]["InstructionKind"],
              item["InsertPosition"]["NickName"],
              item["InsertPosition"]["Helper"],
              item["InsertPosition"]["Direction"]));
          break;
        case Kind::IfThen: {
          IfThenGadget *ifthen = new IfThenGadget(
              kind, item["Module"], item["Function"], item["Type"],
              ArithmeticGadget(
                  this->getKind(item["Cond"]["Kind"].get<std::string>()),
                  item["Module"], item["Function"], item["Cond"]["LHS"],
                  item["Cond"]["LHSPos"], item["Cond"]["RHS"],
                  item["Cond"]["RHSPos"], item["Cond"]["Dst"],
                  item["Cond"]["DstPos"], item["Cond"]["Type"],
                  item["InsertPosition"]["InstructionKind"],
                  item["InsertPosition"]["NickName"],
                  item["InsertPosition"]["Helper"],
                  item["InsertPosition"]["Direction"]),
              item["InsertPosition"]["InstructionKind"],
              item["InsertPosition"]["NickName"],
              item["InsertPosition"]["Helper"],
              item["InsertPosition"]["Direction"]);
          for (auto &thenitem : item["Then"]) {
            switch (this->getKind(thenitem["Kind"].get<std::string>())) {
            case Kind::Assignment: {
              ifthen->thenlist.push_back(new AssignmentGadget(
                  this->getKind(thenitem["Kind"].get<std::string>()),
                  item["Module"], item["Function"], thenitem["Src"],
                  thenitem["SrcPos"], thenitem["Dst"], thenitem["DstPos"],
                  thenitem["Type"], item["InsertPosition"]["InstructionKind"],
                  item["InsertPosition"]["NickName"],
                  item["InsertPosition"]["Helper"],
                  item["InsertPosition"]["Direction"]));
              break;
            }
            case Kind::Arithmetic: {
              ifthen->thenlist.push_back(new ArithmeticGadget(
                  this->getKind(thenitem["Kind"].get<std::string>()),
                  item["Module"], item["Function"], thenitem["LHS"],
                  thenitem["LHSPos"], thenitem["RHS"], thenitem["RHSPos"],
                  thenitem["Dst"], thenitem["DstPos"], thenitem["Type"],
                  item["InsertPosition"]["InstructionKind"],
                  item["InsertPosition"]["NickName"],
                  item["InsertPosition"]["Helper"],
                  item["InsertPosition"]["Direction"]));
              break;
            }
            case Kind::Call: {
              ifthen->thenlist.push_back(new CallGadget(
                  this->getKind(thenitem["Kind"].get<std::string>()),
                  item["Module"], item["Function"], thenitem["Name"],
                  thenitem["Parameter1"], thenitem["Parameter1Pos"],
                  thenitem["Parameter2"], thenitem["Parameter2Pos"],
                  thenitem["Parameter3"], thenitem["Parameter3Pos"],
                  thenitem["Type"], item["InsertPosition"]["InstructionKind"],
                  item["InsertPosition"]["NickName"],
                  item["InsertPosition"]["Helper"],
                  item["InsertPosition"]["Direction"]));
              break;
            }
            default:
              break;
            }
          }
          gadget_list.push_back(ifthen);
          break;
        }
        case Kind::Call:
          gadget_list.push_back(new CallGadget(
              kind, item["Module"], item["Function"], item["Name"],
              item["Parameter1"], item["Parameter1Pos"], item["Parameter2"],
              item["Parameter2Pos"], item["Parameter3"], item["Parameter3Pos"],
              item["Type"], item["InsertPosition"]["InstructionKind"],
              item["InsertPosition"]["NickName"],
              item["InsertPosition"]["Helper"],
              item["InsertPosition"]["Direction"]));
          break;

        default:
          assert(false && "None Kind");
          break;
        }
      }
    }
    // errs() << "Find target function: " <<
    // data["GadgetList"][0]["Kind"].dump();
  } catch (const std::exception &e) {
    errs() << e.what() << '\n';
    exit(-1);
  }
  return gadget_list;
}

Kind GadgetController::getKind(string kindstr) {
  Kind kind = kindstr == "GlobalVariable"  ? Kind::GlobalVariable
              : kindstr == "LocalVariable" ? Kind::LocalVariable
              : kindstr == "Assignment"    ? Kind::Assignment
              : kindstr == "Arithmetic"    ? Kind::Arithmetic
              : kindstr == "IfThen"        ? Kind::IfThen
              : kindstr == "Call"          ? Kind::Call
                                           : Kind::None;
  return kind;
}
// 通过模板装饰Generator的方法扩充 但是好像并不需要 直接继承就行
// 也可使用工厂模式 创建不通类型的Generator
