#pragma once
#include "LLVM.h"
#include <string>
using namespace std;
using namespace llvm;
enum Kind {
  GlobalVariable,
  LocalVariable,
  Assignment,
  Arithmetic,
  IfThen,
  Call,
  None
};

class InsertPoint {
private:
  Instruction *insert_point = nullptr;

public:
  string instkind;
  string nickname;
  string helper;
  string direction;
  InsertPoint(string instkind, string nickname, string helper, string direction)
      : instkind(instkind), nickname(nickname), helper(helper),
        direction(direction) {}
  Instruction *getInsertPoint() { return insert_point; }
  void setInsertPoint(Instruction *point) { this->insert_point = point; }
};

class GadgetToInsert {
public:
  Kind kind;
  string module;
  string function;
  GadgetToInsert(Kind kind, string module, string function)
      : kind(kind), module(module), function(function) {}
  virtual InsertPoint *getPointInfo() = 0;
  virtual void setPointInfo(Instruction *point) = 0;
};

class GlobalVariableGadget : public GadgetToInsert {
public:
  string name;
  string type;

  GlobalVariableGadget(Kind kind, string module, string function, string name,
                       string type)
      : GadgetToInsert(kind, module, function), name(name), type(type) {}
  InsertPoint *getPointInfo() { return nullptr; }
  void setPointInfo(Instruction *point) {}
};

class LocalVariableGadget : public GadgetToInsert {
public:
  string name;
  string type;
  InsertPoint insertpoint;

  LocalVariableGadget(Kind kind, string module, string function, string name,
                      string type, string instkind, string nickname,
                      string helper, string direction)
      : GadgetToInsert(kind, module, function), name(name), type(type),
        insertpoint(instkind, nickname, helper, direction) {}
  InsertPoint *getPointInfo() { return &insertpoint; }
  void setPointInfo(Instruction *point) {
    this->insertpoint.setInsertPoint(point);
  }
};

class AssignmentGadget : public GadgetToInsert {
public:
  string src;
  string srcpos;
  string dst;
  string dstpos;
  string type;
  InsertPoint insertpoint;
  AssignmentGadget(Kind kind, string module, string function, string src,
                   string srcpos, string dst, string dstpos, string type,
                   string instkind, string nickname, string helper,
                   string direction)
      : GadgetToInsert(kind, module, function), src(src), srcpos(srcpos),
        dst(dst), dstpos(dstpos), type(type),
        insertpoint(instkind, nickname, helper, direction) {}
  InsertPoint *getPointInfo() { return &insertpoint; }
  void setPointInfo(Instruction *point) {
    this->insertpoint.setInsertPoint(point);
  }
};

class ArithmeticGadget : public GadgetToInsert {
public:
  string lhs;
  string lhspos;
  string rhs;
  string rhspos;
  string dst;
  string dstpos;
  string type;
  InsertPoint insertpoint;
  ArithmeticGadget(Kind kind, string module, string function, string lhs,
                   string lhspos, string rhs, string rhspos, string dst,
                   string dstpos, string type, string instkind, string nickname,
                   string helper, string direction)
      : GadgetToInsert(kind, module, function), lhs(lhs), lhspos(lhspos),
        rhs(rhs), rhspos(rhspos), dst(dst), dstpos(dstpos), type(type),
        insertpoint(instkind, nickname, helper, direction) {}
  InsertPoint *getPointInfo() { return &insertpoint; }
  void setPointInfo(Instruction *point) {
    this->insertpoint.setInsertPoint(point);
  }
};

class IfThenGadget : public GadgetToInsert {
public:
  ArithmeticGadget cond;
  std::vector<GadgetToInsert *> thenlist;
  string type;
  InsertPoint insertpoint;
  IfThenGadget(Kind kind, string module, string function, string type,
               ArithmeticGadget cond, string instkind, string nickname,
               string helper, string direction)
      : GadgetToInsert(kind, module, function), type(type), cond(cond),
        insertpoint(instkind, nickname, helper, direction) {}
  InsertPoint *getPointInfo() { return &insertpoint; }
  void setPointInfo(Instruction *point) {
    this->insertpoint.setInsertPoint(point);
  }
};

class CallGadget : public GadgetToInsert {
public:
  string name;
  string parameter1;
  string parameter1Pos;
  string parameter2;
  string parameter2Pos;
  string parameter3;
  string parameter3Pos;
  string type;
  InsertPoint insertpoint;
  CallGadget(Kind kind, string module, string function, string name,
             string parameter1, string parameter1Pos, string parameter2,
             string parameter2Pos, string parameter3, string parameter3Pos,
             string type, string instkind, string nickname, string helper,
             string direction)
      : GadgetToInsert(kind, module, function), name(name),
        parameter1(parameter1), parameter1Pos(parameter1Pos),
        parameter2(parameter2), parameter2Pos(parameter2Pos),
        parameter3(parameter3), parameter3Pos(parameter3Pos), type(type),
        insertpoint(instkind, nickname, helper, direction) {}
  InsertPoint *getPointInfo() { return &insertpoint; }
  void setPointInfo(Instruction *point) {
    this->insertpoint.setInsertPoint(point);
  }
};
