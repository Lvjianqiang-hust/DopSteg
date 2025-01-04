#include "GadgetGenerator.h"

using namespace llvm;

void GadgetGenerator::setInsertPoint(Instruction *inst) {
  this->builder->SetInsertPoint(inst);
}
void GadgetGenerator::setInsertPoint(BasicBlock *bb) {
  this->builder->SetInsertPoint(bb);
}
void GadgetGenerator::insertGlobalVariables(StringRef name, StringRef typestr) {

  //区分是本地变量还是外部变量的方法在于是否具有初始化器
  bool external = false;
  if (typestr.contains("extern")) {
    typestr = typestr.substr(7);
    external = true;
  }
  if (typestr == "char*") {
    auto *type = builder->getInt8PtrTy();
    M->getOrInsertGlobal(name, type);
    auto *x = M->getNamedGlobal(name);
    x->setDSOLocal(true);
    if (!external) {
      x->setLinkage(GlobalValue::CommonLinkage);
      x->setInitializer(ConstantPointerNull::get(type));
    }
  } else if (typestr == "int*") {
    auto *type = builder->getInt32Ty()->getPointerTo();
    M->getOrInsertGlobal(name, type);
    auto *x = M->getNamedGlobal(name);
    x->setDSOLocal(true);
    if (!external) {
      x->setLinkage(GlobalValue::CommonLinkage);
      x->setInitializer(ConstantPointerNull::get(type));
    }
  } else if (typestr.contains('[')) {
    // 1. Initialize chars vector 默认初始化为0
    IntegerType *basetype = nullptr;
    if (typestr.startswith("char")) {
      basetype = builder->getInt8Ty();
    } else if (typestr.startswith("int")) {
      basetype = builder->getInt32Ty();
    }
    
    // 2. Initialize the string from the characters
    auto lBracket = typestr.find("[");
    auto rBracket = typestr.find("]");
    StringRef sizeOfArray =
        typestr.substr(lBracket + 1, rBracket - lBracket - 1);
    auto arrayType = llvm::ArrayType::get(basetype, std::stoi(sizeOfArray));
    auto *initializer = ConstantAggregateZero::get(arrayType);
    // 3. Create the declaration statement
    auto globalDeclaration =
        (llvm::GlobalVariable *)M->getOrInsertGlobal(name, arrayType);
    globalDeclaration->setDSOLocal(true);
    // globalDeclaration->setConstant(false);
    if (!external) {
      globalDeclaration->setLinkage(GlobalValue::CommonLinkage);
      globalDeclaration->setInitializer(initializer);
    }
    //globalDeclaration->setLinkage(GlobalValue::CommonLinkage);
    // globalDeclaration->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    // 4. Return a cast to an i8*
    // return
    // ConstantExpr::getBitCast(globalDeclaration,charType->getPointerTo());
  } else if (typestr == "int") {
    auto *type = builder->getInt32Ty();
    M->getOrInsertGlobal(name, type);
    auto *x = M->getNamedGlobal(name);
    x->setDSOLocal(true);
    if (!external) {
      x->setLinkage(GlobalValue::CommonLinkage);
      x->setInitializer(builder->getInt32(0));
    }
  } else {
    assert(false && "only support char* int* char[NUM] int");
  }
}
void GadgetGenerator::insertLocalVariables(StringRef name, StringRef typestr) {
  if (typestr == "int" || typestr == "int32") {
    auto *type = builder->getInt32Ty();
    Value *local_inuse = builder->CreateAlloca(type, nullptr, name);
    // local_inuse = builder->CreateLoad(local_inuse);
    local_inuse->setName(name);
    builder->CreateStore(builder->getInt32(0), local_inuse);
  } else if (typestr == "char*") {
    auto *type = builder->getInt8PtrTy();
    Value *local_pointer = builder->CreateAlloca(type, nullptr, name);
    local_pointer->setName(name);
    builder->CreateStore(ConstantPointerNull::get(type), local_pointer);
  } else if (typestr.contains('[')) {
    // %buffer = alloca [200 x i8], align 16
    // 1. Initialize chars vector 默认初始化为0
    IntegerType *basetype = nullptr;
    if (typestr.startswith("char")) {
      basetype = builder->getInt8Ty();
    } else if (typestr.startswith("int")) {
      basetype = builder->getInt32Ty();
    }
    auto *initializer = ConstantAggregateZero::get(basetype);
    // 2. Initialize the string from the characters
    auto lBracket = typestr.find("[");
    auto rBracket = typestr.find("]");
    StringRef sizeOfArray =
        typestr.substr(lBracket + 1, rBracket - lBracket - 1);
    auto arrayType = llvm::ArrayType::get(basetype, std::stoi(sizeOfArray));
    // 3. Create the declaration statement
    auto local_array = builder->CreateAlloca(arrayType, nullptr, name);
    builder->CreateStore(ConstantAggregateZero::get(arrayType), local_array);
  } else {
    assert(false && "only support int char*");
  }
}
void GadgetGenerator::insertAssignment(StringRef src, StringRef srcpos,
                                       StringRef dst, StringRef dstpos,
                                       StringRef type) {
  Value *src_variable = this->getVariableValuePointer(src, srcpos);
  Value *dst_variable = this->getVariableValuePointer(dst, dstpos, true);
  // 当两个类型不一致时，尝试转换到一致类型
  if (src_variable->getType()->isIntegerTy()) {
    // errs()<<dst_variable->getType()->getPointerElementType()->getIntegerBitWidth()<<'\n';
    // errs()<<src_variable->getType()->getIntegerBitWidth()<<'\n';
    auto src_width = src_variable->getType()->getIntegerBitWidth();
    if (dst_variable->getType()->isIntegerTy()) {
      auto dst_width = dst_variable->getType()->getIntegerBitWidth();
      if (src_width > dst_width) {
        src_variable = builder->CreateTrunc(
            src_variable, dst_variable->getType()->getPointerElementType());
      }
      else if (src_width < dst_width){
        src_variable = builder->CreateSExt(
            src_variable, dst_variable->getType()->getPointerElementType());
      }
    } else if (dst_variable->getType()->isPointerTy() &&
               !dst_variable->getType()
                    ->getPointerElementType()
                    ->isIntegerTy()) {
      // %41 = load i32*, i32** @deref, align 8
      // %42 = load i32, i32* %41, align 4
      // %conv58 = sext i32 %42 to i64
      // %43 = inttoptr i64 %conv58 to i32*
      // store i32* %43, i32** @deref, align 8
      // 忽略宽度问题 默认  全部为32位情况
      src_variable =
          builder->CreateIntToPtr(src_variable, dst_variable->getType()->getPointerElementType());
    } else {
      auto dst_width = dst_variable->getType()
                           ->getPointerElementType()
                           ->getIntegerBitWidth();
      if (src_width > dst_width) {
        src_variable = builder->CreateTrunc(
             src_variable, dst_variable->getType()->getPointerElementType());
      }
      else if (src_width < dst_width){
        src_variable = builder->CreateSExt(
            src_variable, dst_variable->getType()->getPointerElementType());
      }
    }
  }

  assert(src_variable && dst_variable && "src and pos must exist");
  builder->CreateStore(src_variable, dst_variable);
}
Value *GadgetGenerator::insertArithmetic(StringRef lhs, StringRef lhspos,
                                         StringRef rhs, StringRef rhspos,
                                         StringRef dst, StringRef dstpos,
                                         StringRef type) {
  Value *lhs_variable = this->getVariableValuePointer(lhs, lhspos);
  Value *rhs_variable = this->getVariableValuePointer(rhs, rhspos);
  Value *dst_variable = this->getVariableValuePointer(dst, dstpos, true);

  assert(lhs_variable && rhs_variable && "lhs and rhs must be exist");
  if (lhs_variable->getType()->isPointerTy() &&
      rhs_variable->getType()->isIntegerTy() && type == "+") {
    //对全局数组指针的引用操作 参照 https://llvm.org/docs/GetElementPtr.html#id5
    if (auto lhs_global = dyn_cast<GlobalVariable>(lhs_variable)) {
      if (lhs_global->getType()->getElementType()->isArrayTy()) {
        Value *i32zero = builder->getInt32(0);
        Value *indices[2] = {i32zero, rhs_variable};
        lhs_variable = builder->CreateInBoundsGEP(
            lhs_variable->getType()->getPointerElementType(), lhs_variable,
            indices);
        builder->CreateStore(lhs_variable, dst_variable);
      } else if (lhs_global->getType()->getElementType()->isPointerTy()) {
        lhs_variable = builder->CreateLoad(lhs_variable);
        lhs_variable = builder->CreateInBoundsGEP(
            lhs_variable->getType()->getPointerElementType(), lhs_variable,
            rhs_variable);
        builder->CreateStore(lhs_variable, dst_variable);
      }
    } else {
      //旧有的处理逻辑 会出问题 如果遇到局部变量的问题 先改这个
      lhs_variable = builder->CreateInBoundsGEP(
          lhs_variable->getType()->getPointerElementType(), lhs_variable,
          rhs_variable);
      builder->CreateStore(lhs_variable, dst_variable);
    }
  } else if (dst == "null" && dstpos == "null") {
    if (type == "!=") {
      // lhs_variable = builder->CreateLoad(lhs_variable);
      dst_variable = builder->CreateICmpNE(lhs_variable, rhs_variable);
    } else if (type == "==") {
      dst_variable = builder->CreateICmpEQ(lhs_variable, rhs_variable);
    } else {
      assert(false && "only support != cmp");
    }
  } else if (lhs_variable->getType()->isIntegerTy() &&
             rhs_variable->getType()->isIntegerTy()) {
    switch (type[0]) {
    case '+':
      lhs_variable =
          builder->CreateAdd(lhs_variable, rhs_variable, "", false, true);
      builder->CreateStore(lhs_variable, dst_variable);
      break;
    case '%':
      lhs_variable = builder->CreateSRem(lhs_variable, rhs_variable, "");
      builder->CreateStore(lhs_variable, dst_variable);
      break;
    case '*':
      lhs_variable =
          builder->CreateMul(lhs_variable, rhs_variable, "", false, true);
      builder->CreateStore(lhs_variable, dst_variable);
      break;
    default:
      assert(false && "only support + % *");
      break;
    }
  } else {
    assert(false && "only support char* type");
  }
  return dst_variable;
}
Value *GadgetGenerator::getVariableValuePointer(StringRef variablestr,
                                                StringRef variablepos,
                                                bool is_left_value) {
  Value *variable = nullptr;
  if (variablepos == "GlobalVariable") {
    if (variablestr.find("[") != StringRef::npos) {
      auto lBracket = variablestr.find("[");
      auto rBracket = variablestr.find("]");
      StringRef indexofarray =
          variablestr.substr(lBracket + 1, rBracket - lBracket - 1);
      variablestr = variablestr.substr(0, lBracket);
      variable = M->getNamedGlobal(variablestr);
      GlobalVariable *global_variable = dyn_cast<GlobalVariable>(variable);
      if (global_variable->getType()->getElementType()->isAggregateType()) {
        Value *i32zero = builder->getInt32(0);
        Value *indices[2] = {i32zero,
                             builder->getInt32(std::stoi(indexofarray))};
        variable = builder->CreateInBoundsGEP(variable, indices);
        variable = builder->CreateLoad(variable);
      } else if (global_variable->getType()->isPointerTy()) {
        variable = builder->CreateLoad(variable);
        variable = builder->CreateInBoundsGEP(
            variable->getType()->getPointerElementType(), variable,
            builder->getInt32(std::stoi(indexofarray)));
        variable = builder->CreateLoad(variable);
      }
    } else if (variablestr.startswith("*")) {
      variablestr = variablestr.substr(1);
      variable = M->getNamedGlobal(variablestr);
      variable = builder->CreateLoad(variable);
      if (!is_left_value) {
        variable = builder->CreateLoad(variable);
      }
    } else {
      auto global_variable = M->getNamedGlobal(variablestr);
      if (global_variable->getType()->getElementType()->isIntegerTy() &&
          !is_left_value) {
        variable = builder->CreateLoad(global_variable);
        // variable = global_variable;
      } else {
        //暂时不处理
        variable = global_variable;
      }
    }
  } else if (variablepos == "LocalVariable") {
    if (variablestr.startswith("*")) {
      variablestr = variablestr.substr(1);
      //为了适应对指针的多次解引用，使用递归解决问题---暂不执行
      auto inst = this->getLocalVariablePointer(variablestr);
      variable = inst;
      if (AllocaInst *alloca_inst = dyn_cast<AllocaInst>(inst)) {
        if (alloca_inst->getType()->getElementType()->isAggregateType()) {
          Value *i32zero = builder->getInt32(0);
          Value *indices[2] = {i32zero, i32zero};
          //%arraydecay2 = getelementptr inbounds [200 x i8], [200 x i8]*
          //%buffer, i64 0, i64 0
          variable = builder->CreateInBoundsGEP(alloca_inst, indices);
        } else if (alloca_inst->getType()->getElementType()->isPointerTy()) {
          variable = builder->CreateLoad(inst);
        }
      } else {
        variable = builder->CreateLoad(inst);
      }
      if (!is_left_value) {
        variable = builder->CreateLoad(variable);
      }
    } else if (variablestr.startswith("&")) {
      // https://penguin-wenyang.wang/2018/04/14/LLVM-Pass-add-process/
      variablestr = variablestr.substr(1);
      if (variablestr.contains(".")) {
        auto tmp = variablestr.split(".");
        auto struct_str = tmp.first;
        auto elem_str = tmp.second;
        int offset = 0;
        // 除了结构体成员为数组的情况
        if (elem_str.contains('[')) {
          auto lBracket = elem_str.find("[");
          auto rBracket = elem_str.find("]");
          StringRef indexofarray =
              elem_str.substr(lBracket + 1, rBracket - lBracket - 1);
          offset = std::stoi(indexofarray);
          elem_str = elem_str.substr(0, lBracket);
        }
        AllocaInst *struct_inst =
            (AllocaInst *)this->getLocalVariablePointer(struct_str);
        // struct_inst->print(errs());
        //  使用debug信息得到结构的对应成员的偏移量
        int distance = 0;
        DebugInfoFinder Finder;
        Finder.processModule(*this->M);
        bool find = false;
        for (DebugInfoFinder::type_iterator i = Finder.types().begin(),
                                            e = Finder.types().end();
             i != e; ++i) {
          if (find)
            break;
          if (auto S = dyn_cast<DICompositeType>(*i))
            if (S->getName() == struct_inst->getName())
              for (auto elem : S->getElements()) {
                if (DIDerivedType *ele = dyn_cast<DIDerivedType>(elem)) {
                  // ele->print(dbgs());
                  // dbgs() << '\n';
                  if (ele->getName() == elem_str) {
                    find = true;
                    // dbgs() << "find" << distance << '\n';
                    break;
                  } else {
                    distance += 1;
                  }
                }
              }
        }
        // 准备就绪
        // find mac prt
        Value *indices[2] = {builder->getInt32(0), builder->getInt32(distance)};
        auto struct_mem = builder->CreateInBoundsGEP(struct_inst, indices);
        indices[1] = builder->getInt32(offset);
        variable = builder->CreateInBoundsGEP(struct_mem, indices);
        if (is_left_value) {
          //以&开头，所以，当为目的操作数时，应当再load一次
          // 不保证正确
          variable = builder->CreateLoad(variable);
        } else {
          //当为源操作数时，&符号，恰好抵消一次load操作
        }
      } else {
        assert(false && "only support struct data");
      }
    } else if (variablestr.contains('[')) {
      auto lBracket = variablestr.find("[");
      auto rBracket = variablestr.find("]");
      StringRef indexofarray =
          variablestr.substr(lBracket + 1, rBracket - lBracket - 1);
      // errs() << "12312312" << indexofarray << '\n';
      variablestr = variablestr.substr(0, lBracket);
      auto inst = this->getLocalVariablePointer(variablestr);
      AllocaInst *alloca_inst = dyn_cast<AllocaInst>(inst);
      if (alloca_inst->getType()->getElementType()->isAggregateType()) {
        Value *i32zero = builder->getInt32(0);
        Value *indices[2] = {i32zero,
                             builder->getInt32(std::stoi(indexofarray))};
        variable = builder->CreateInBoundsGEP(inst, indices);
        if (!is_left_value)
          variable = builder->CreateLoad(variable);
      } else if (alloca_inst->getType()->isPointerTy()) {
        variable = builder->CreateLoad(inst);
        variable = builder->CreateInBoundsGEP(
            variable->getType()->getPointerElementType(), variable,
            builder->getInt32(std::stoi(indexofarray)));
        if (!is_left_value)
          variable = builder->CreateLoad(variable);
      } else {
        assert(false && "something wrong at index of local array");
      }
    } else if (variablestr.contains(".")) {
      auto tmp = variablestr.split(".");
      auto struct_str = tmp.first;
      auto elem_str = tmp.second;
      // 除了结构体成员为数组的情况
      if (elem_str.contains('[')) {
        assert(false && "do not support array ");
      }
      AllocaInst *struct_inst =
          (AllocaInst *)this->getLocalVariablePointer(struct_str);
      // struct_inst->print(errs());
      //  使用debug信息得到结构的对应成员的偏移量
      int distance = 0;
      DebugInfoFinder Finder;
      Finder.processModule(*this->M);
      bool find = false;
      for (DebugInfoFinder::type_iterator i = Finder.types().begin(),
                                          e = Finder.types().end();
           i != e; ++i) {
        if (find)
          break;
        if (auto S = dyn_cast<DICompositeType>(*i))
          if (S->getName() == struct_inst->getName())
            for (auto elem : S->getElements()) {
              if (DIDerivedType *ele = dyn_cast<DIDerivedType>(elem)) {
                // ele->print(dbgs());
                // dbgs() << '\n';
                if (ele->getName() == elem_str) {
                  find = true;
                  // dbgs() << "find" << distance << '\n';
                  break;
                } else {
                  distance += 1;
                }
              }
            }
      }
      // 准备就绪
      // find mac ptr
      Value *indices[2] = {builder->getInt32(0), builder->getInt32(distance)};
      auto struct_mem = builder->CreateInBoundsGEP(struct_inst, indices);
      // indices[1] = builder->getInt32(offset);
      variable = struct_mem;
      if (is_left_value) {
        //以&开头，所以，当为目的操作数时，应当再load一次
        // 不保证正确
        variable = builder->CreateLoad(variable);
      } else {
        //当为源操作数时，&符号，恰好抵消一次load操作
      }
    } else {
      auto inst = this->getLocalVariablePointer(variablestr);
      if (AllocaInst *alloca_inst = dyn_cast<AllocaInst>(inst)) {
        if (alloca_inst->getName() == variablestr) {
          if (alloca_inst->getType()->getElementType()->isAggregateType()) {
            Value *i32zero = builder->getInt32(0);
            Value *indices[2] = {i32zero, i32zero};
            //%arraydecay2 = getelementptr inbounds [200 x i8], [200 x i8]*
            //%buffer, i64 0, i64 0
            variable = builder->CreateInBoundsGEP(alloca_inst, indices);
            // variable = alloca_inst;
          } else {
            if (is_left_value) {
              variable = alloca_inst;
            } else
              variable = builder->CreateLoad(alloca_inst);
          }
        }
      }
      else if (GetElementPtrInst* gepi_inst = dyn_cast<GetElementPtrInst>(inst)) {
        //lm-3.28
        if (is_left_value) {
            variable = gepi_inst;
        } else
            variable = builder->CreateLoad(gepi_inst);
      }
    }
  } else if (variablepos == "int") {
    variable = builder->getInt32(std::stoi(variablestr));
  } else if (variablepos == "nullptr") {
    variable = ConstantPointerNull::get(builder->getInt8PtrTy());
  }
  return variable;
}
Instruction *GadgetGenerator::getLocalVariablePointer(StringRef variablestr) {
  Instruction *variable = nullptr;
  bool find = false;
  for (auto &bb : *F) {
    if (find)
      break;
    for (auto &inst : bb) {
      if (find)
        break;
      if (AllocaInst *alloca_inst = dyn_cast<AllocaInst>(&inst)) {
        if (alloca_inst->getName() == variablestr) {
          variable = &inst;
          find = true;
          return variable;
        }
      }
      else if (GetElementPtrInst* gepi_inst = dyn_cast<GetElementPtrInst>(&inst)) {
        //lm-3.28
        if (gepi_inst->getName() == variablestr) {
          variable = &inst;
          find = true;
          return variable;
        }
      }
    }
  }
  if (variable == nullptr) {
    for (auto arg = F->arg_begin(); arg != F->arg_end(); ++arg) {
      auto tmp = &*arg;
      if (tmp->getName() == variablestr) {
        //依赖于ir命名特性 寻找.addr的地址
        StringRef tmpstr = variablestr.str() + ".addr";
        variable = this->getLocalVariablePointer(tmpstr);
        break;
      }
    }
  }
  return variable;
}
Instruction *GadgetGenerator::insertIfThenBlock(Value *cond,
                                                Instruction *split_before) {
  auto *then_terminator = SplitBlockAndInsertIfThen(cond, split_before, false);
  return then_terminator;
}
Value *GadgetGenerator::insertCall(StringRef name, StringRef param1,
                                   StringRef param1pos, StringRef param2,
                                   StringRef param2pos, StringRef param3,
                                   StringRef param3pos) {
  //  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %12, i8* align 16 %13,
  //  i64 200, i1 false)
  if (name == "memcpy") {
    Value *param1_varaiable = this->getVariableValuePointer(param1, param1pos);
    Value *param2_varaiable = this->getVariableValuePointer(param2, param2pos);
    Value *param3_varaiable = this->getVariableValuePointer(param3, param3pos);
    assert(param1_varaiable && param2_varaiable && param3_varaiable &&
           "p1 p2 p3 must exist");
    // param1_varaiable->print(errs(), true);
    // param2_varaiable->print(errs(), true);
    // param3_varaiable->print(errs(), true);

    /// If the pointers aren't i8*, they will be converted.
    Value *res = builder->CreateMemCpy(
        param1_varaiable,
        param1_varaiable->getPointerAlignment(M->getDataLayout()),
        param2_varaiable,
        param2_varaiable->getPointerAlignment(M->getDataLayout()),
        param3_varaiable);
    return res;
  } else {
    assert(false && "only support memcpy");
  }
  return nullptr;
}

void GadgetGenerator::insertLoopGadgets() {
  auto *insert_block = builder->GetInsertBlock();
  // BasicBlock *newBB =
  //     BasicBlock::Create(context, "", insert_block->getParent(),
  //     insert_block);
  BasicBlock *insertBB =
      SplitBlock(insert_block, insert_block->getFirstNonPHIOrDbgOrLifetime());
  builder->SetInsertPoint(
      insertBB->getPrevNode()->getFirstNonPHIOrDbgOrLifetime());
  PHINode *i_pn = builder->CreatePHI(builder->getInt32Ty(), 0, "i");
  // PHINode::Create(builder->getInt32Ty(), 0, "i",
  //                 insert_block->getFirstNonPHIOrDbgOrLifetime());
  i_pn->addIncoming(builder->getInt32(0), insert_block);

  Value *i =
      builder->CreateAdd(i_pn, builder->getInt32(1), "", true, true); // i++

  i_pn->addIncoming(i, insertBB);

  // Instruction *i_icmp =
  //     ICmpInst::Create(Instruction::OtherOps::ICmp,
  //     CmpInst::Predicate::ICMP_EQ,
  //                      i, builder->getInt32(16));
  Value *i_icmp = builder->CreateICmpEQ(i, builder->getInt32(16),
                                        "i_icmp"); // %12 = icmp eq i64 %4, 16
  // builder->Insert(i_icmp);
  // Instruction *insertBB = SplitBlockAndInsertIfThen(
  //     i_cond, insert_block->getFirstNonPHIOrDbgOrLifetime(), false);
  BasicBlock *next = insertBB->getNextNode();
  // builder->SetInsertPoint(i_icmp->getNextNode());
  // Value *i_br =
  //     builder->CreateCondBr(i_icmp, next,
  //                           insertBB); // br i1 %12, label %13, label %2
  // insertBB->print(errs());
}
