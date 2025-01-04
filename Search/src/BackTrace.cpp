#include "BackTrace.h"

std::map<Instruction*,Gadget> BackTrace::interesting_storeinst;

BackTrace::BackTrace(Instruction* I,FuncType t, bool isc):isavailable(false),gadget(I),interesting_flag(std::vector<bool>(2,false)),
isavail_flag(std::vector<bool>(2,false)),func_type(t),iscall(isc),has_a_call(false),is_arithmetic(false),special_func(""),pre_inst(nullptr){
    this->analyzeInstruction();
}

void BackTrace::analyzeInstruction(){
    if(this->iscall){
        CallInst* CI=dyn_cast<CallInst>(this->gadget.inst);
        special_func=CI->getCalledFunction()->getName();
        if(special_func=="printf"){
            //从第2个参数开始
            for(int i=1;i<CI->getNumArgOperands();i++){
                this->wait2visit.push({CI->getArgOperand(i),OperandType::SRC});
            }
            this->isavail_flag[1]=this->interesting_flag[1]=true;   
        }
        else if(special_func=="fprintf"){
            //从第3个参数开始
            for(int i=2;i<CI->getNumArgOperands();i++){
                this->wait2visit.push({CI->getArgOperand(i),OperandType::SRC});
            }
            this->isavail_flag[1]=this->interesting_flag[1]=true;
        }
        else if(special_func=="putc"||special_func=="fputc"){
            //第1个参数
            this->wait2visit.push({CI->getArgOperand(0),OperandType::SRC});
            this->isavail_flag[1]=this->interesting_flag[1]=true;
        }
        else if(special_func=="send"||special_func=="write"||special_func=="sendmsg"){
            //第2个参数
            this->wait2visit.push({CI->getArgOperand(1),OperandType::SRC});
            this->isavail_flag[1]=this->interesting_flag[1]=true;
        }
        else if(special_func=="vsnprintf"){
            //遇到vsnprintf时，一定是与参数有关，将目的buffer记录成src，具体插入位置应该设置为vsnprintf之后
            this->wait2visit.push({CI->getArgOperand(0),OperandType::DST});
            this->wait2visit.push({CI->getArgOperand(2),OperandType::SRC});
        }
        else if(special_func=="snprintf"){
            this->wait2visit.push({CI->getArgOperand(0),OperandType::DST});
            for(int i=2;i<CI->getNumArgOperands();i++){
                this->wait2visit.push({CI->getArgOperand(i),OperandType::SRC});
            }            
        }
        else if(Global::copy_func.count(special_func)){
            //前两个参数
            this->wait2visit.push({CI->getArgOperand(0),OperandType::DST});
            this->wait2visit.push({CI->getArgOperand(1),OperandType::SRC});
        }
    }
    else{
        this->wait2visit.push({this->gadget.inst,OperandType::NONE});
    }

    while(!this->wait2visit.empty()){
        //整个溯源过程
        Value* ptr=this->wait2visit.top().first;
        this->op_type=this->wait2visit.top().second;
        this->wait2visit.pop();
        if(ptr==nullptr)
            continue;
        
        if(Instruction* I=dyn_cast<Instruction>(ptr)){
            visit(*I);
            //errs() << getString(*I) + "\n";
            //保存上一条指令，即在插入赋值语句时的具体位置信息。
            if(this->special_func=="vsnprintf"||this->special_func=="snprintf"){
                this->pre_inst=this->gadget.inst;
            }
            else{
                this->pre_inst=I;
            }
            //保存溯源过程
            if(this->op_type==OperandType::SRC){
                if(this->gadget.src_inst.size()<=BACKSRCNUM)
                    this->gadget.src_inst.insert(this->gadget.src_inst.begin(),I);
            }
            else if(this->op_type==OperandType::DST){
                if(this->gadget.dst_inst.size()<=BACKSRCNUM)
                    this->gadget.dst_inst.insert(this->gadget.dst_inst.begin(),I);
            }
        }
        else if(dyn_cast<Constant>(ptr)){
            if(ConstantExpr *CE = dyn_cast<ConstantExpr>(ptr)){
                //不经常遇到这种情况
                switch (CE->getOpcode()){
                    case Instruction::GetElementPtr:
                    case Instruction::BitCast:
                    case Instruction::PtrToInt:
                    case Instruction::IntToPtr:
                        this->wait2visit.push({CE->getOperand(0),this->op_type});
                        break;
                    case Instruction::Add:
                    case Instruction::Sub:
                    case Instruction::Mul:
                        for(auto opi = CE->op_begin(); opi != CE->op_end(); opi++){
                            this->wait2visit.push({*opi,this->op_type});
                        }
                        break;
                    default:
                        Global::logger->log("UNKNOWN CE:" + getString(*CE) + "\n");
                        exit(-1);
                }
            }
            else if(GlobalVariable* GV=dyn_cast<GlobalVariable>(ptr)){
                Global::logger->log("FOUND Global Constant:\t" + getString(*GV) + "\n");
                std::string type_str=getString(*GV->getType());
                if(!this->iscall||this->iscall&&(type_str=="i8**"||type_str.find("i8]*")!=std::string::npos)){
                    if(this->op_type==OperandType::SRC){
                        this->gadget.src[GV]=this->pre_inst;
                    }
                    else{
                        this->gadget.dst[GV]=this->pre_inst;
                    }
                    this->isavail_flag[this->op_type==OperandType::SRC?0:1]=this->interesting_flag[this->op_type==OperandType::SRC?0:1]=true;
                }
            }
            else{
                Global::logger->log("IGNORE Constant:\t" + getString(*ptr) + "\n");
            }
        }
        else if(Argument *A = dyn_cast<Argument>(ptr)){          
            Global::logger->log("FOUND Argument:\t" + getString(*A) + "\n");       
            if(this->op_type==OperandType::SRC){
                this->gadget.src[A]=this->pre_inst;
            }
            else{
                this->gadget.dst[A]=this->pre_inst;
            }
            if(getString(*A->getType()).find("i8*")!=std::string::npos){
                this->interesting_flag[this->op_type==OperandType::SRC?0:1]=true;
                CallInst* CInst=dyn_cast<CallInst>(this->gadget.inst);
                if( this->func_type == FuncType::CHILD || (CInst&&Global::print_func.find(CInst->getCalledFunction()->getName())!=Global::print_func.end())){
                    //对IO函数gadget放宽条件，如果参数与argument有关，也作为gadget
                    //CHILD函数的参数也是需要关注的
                    this->isavail_flag[this->op_type==OperandType::SRC?0:1]=true;
                }
            }
        }
        else{
            Global::logger->log("UNKNOWN Value:\t" + getString(*ptr) + "\n");
            exit(-1);
        }
    }

    if(this->interesting_flag[0]){//如果只有源操作数是interesting，也需要记录，方便后续的分析
        if(this->gadget.src_inst.size()<=BACKSRCNUM&&this->gadget.dst_inst.size()<=BACKSRCNUM){
            BackTrace::interesting_storeinst[this->gadget.inst]=Gadget(this->gadget.inst,this->gadget.src_inst,this->gadget.dst_inst,this->gadget.src,this->gadget.dst);
        }
    }
    this->isavailable=(this->interesting_flag[0]&&this->interesting_flag[1]&&this->isavail_flag[0]&&this->isavail_flag[1]);

    //算数运算要精准
    //常量算有效源
    //简单的局部变量算有效源
    //经过复杂运算的变量不是有效源
    if(this->is_arithmetic&&this->has_a_call){
        this->isavailable=false;
    }
}

bool BackTrace::isInteresting(){
    if(this->isavailable){
        //要保证源和目的操作数不全是字符串常量，否则不是gadget
        bool flag[]={false,false};
        for(auto x:this->gadget.src){
            if(getString(*(x.first)).find("@.str")==std::string::npos){
                flag[0]=true;
                break;
            }
        }
        for(auto x:this->gadget.dst){
            if(getString(*(x.first)).find("@.str")==std::string::npos){
                flag[1]=true;
                break;
            }
        }
        if(flag[0]&&flag[1]||flag[0]&&this->gadget.dst.size()==0){
            return true;
        }
        else{
            return false;
        }
    }
    return false;
}

void BackTrace::visitUnaryInstruction(UnaryInstruction &I){
    Global::logger->log("visitUnaryInstruction:\t"+getString(I)+"\n");
    if(I.getNumOperands()==1){
        this->wait2visit.push({I.getOperand(0),this->op_type});
    }
    else{
        exit(-1);
    }
}
void BackTrace::visitInstruction(Instruction &I){
    Global::logger->log("visitInstruction:\t"+ getString(I) + "\n");
    exit(-1);
}       

void BackTrace::visitLoadInst(LoadInst &I){
    Global::logger->log("visitLoadInst:\t"+ getString(I) + "\n");
    this->wait2visit.push({I.getPointerOperand(),this->op_type});
}

void BackTrace::visitStoreInst(StoreInst &I){
    Global::logger->log("visitStoreInst:\t" + getString(I) + "\n");
    this->wait2visit.push({I.getPointerOperand(),this->op_type==OperandType::NONE?OperandType::DST:this->op_type});
    this->wait2visit.push({I.getValueOperand(),this->op_type==OperandType::NONE?OperandType::SRC:this->op_type});
}

void BackTrace::visitAllocaInst(AllocaInst &I){
    Global::logger->log("visitAllocaInst:\t" + getString(I) + "\n");
    if(this->func_type==FuncType::OVERFLOWFUNC||this->is_arithmetic||this->special_func=="vsnprintf"||this->special_func=="snprintf"){
        //在漏洞函数中局部变量可控；
        //由于算数gadgets数量较少，即便不是漏洞函数中的，也考虑。可以通过插入赋值语句来使用。
        
        if(this->func_type==FuncType::OVERFLOWFUNC){
            //在漏洞函数中，如果是参数则退出
            auto inst_str=getString(I);
            auto name = inst_str.substr(3,inst_str.find_first_of(' ',3)-3);
            if(name.find(".addr")!=std::string::npos){
                return;
            }
        }

        this->interesting_flag[this->op_type==OperandType::SRC?0:1]=true;
        this->isavail_flag[this->op_type==OperandType::SRC?0:1]=true;
        if(this->op_type==OperandType::SRC){
            this->gadget.src[&I]=this->pre_inst;
        }
        else{
            this->gadget.dst[&I]=this->pre_inst;
        }
    }
    else{
        //将SI或CI对应的溯源指令加入到当前的溯源指令中
        auto FoundInterestingSrc=[&I,this](Value::use_iterator it,Instruction* SI){
            this->interesting_flag[this->op_type==OperandType::SRC?0:1]=true;

            std::vector<Instruction*> src_ins=BackTrace::interesting_storeinst[SI].src_inst;

            if(this->op_type==OperandType::SRC){
                bool flag{false};
                CallInst* CI = dyn_cast<CallInst>(SI);
                if(CI&&CI->getCalledFunction()&&(CI->getCalledFunction()->getName()=="vsnprintf"||CI->getCalledFunction()->getName()=="snprintf")){
                    //遇到vsnprintf时，将插装ASSIGNMENT的目的操作数是vsnprintf的dst
                    flag = true;
                }
                if(flag){
                    this->gadget.src.insert(BackTrace::interesting_storeinst[SI].dst.begin(),BackTrace::interesting_storeinst[SI].dst.end());
                }
                else{
                    this->gadget.src.insert(BackTrace::interesting_storeinst[SI].src.begin(),BackTrace::interesting_storeinst[SI].src.end());
                }

                if(this->gadget.src_inst.size()<=BACKSRCNUM){
                    if(flag){
                        std::vector<Instruction*> dst_ins=BackTrace::interesting_storeinst[SI].dst_inst;
                        this->gadget.src_inst.insert(this->gadget.src_inst.begin(),CI);
                        this->gadget.src_inst.insert(this->gadget.src_inst.begin(),dst_ins.begin(),dst_ins.end());                        
                    }
                    else{
                        this->gadget.src_inst.insert(this->gadget.src_inst.begin(),SI);
                        this->gadget.src_inst.insert(this->gadget.src_inst.begin(),src_ins.begin(),src_ins.end());
                    }
                }

                for(auto x:this->gadget.src){
                    Argument* isArg=dyn_cast<Argument>(x.first);
                    CallInst* CInst=dyn_cast<CallInst>(this->gadget.inst);
                    if((CInst&&Global::print_func.find(CInst->getCalledFunction()->getName())!=Global::print_func.end())
                    || (isArg&&this->func_type==FuncType::CHILD)){
                        this->isavail_flag[0]=true;
                        break;
                    }
                }
            }
            else if(this->op_type==OperandType::DST){
                this->gadget.dst.insert(BackTrace::interesting_storeinst[SI].src.begin(),BackTrace::interesting_storeinst[SI].src.end());
                if(this->gadget.dst_inst.size()<=BACKSRCNUM){
                    this->gadget.dst_inst.insert(this->gadget.dst_inst.begin(),SI);
                    this->gadget.dst_inst.insert(this->gadget.dst_inst.begin(),src_ins.begin(),src_ins.end());
                }

                for(auto x:this->gadget.dst){
                    Argument* isArg=dyn_cast<Argument>(x.first);
                    CallInst* CInst=dyn_cast<CallInst>(this->gadget.inst);
                    if((CInst&&Global::print_func.find(CInst->getCalledFunction()->getName())!=Global::print_func.end())
                    || (isArg&&this->func_type==FuncType::CHILD)){
                        this->isavail_flag[1]=true;
                        break;
                    }
                }
            }
        };

        //判断当前CI或SI指令的源是否可控
        auto checkCISI = [this,FoundInterestingSrc](Instruction* I,Value::use_iterator it,StoreInst* SI,CallInst* CI)->bool{

            if(CI&&CI->getCalledFunction()&&Global::copy_func.find(CI->getCalledFunction()->getName())!=Global::copy_func.end()&&CI->getArgOperand(0)==I){
                FoundInterestingSrc(it,CI);
                return true;
            }
            else if(SI&&SI->getPointerOperand()==I&&BackTrace::interesting_storeinst.find(SI)!=BackTrace::interesting_storeinst.end()){
                FoundInterestingSrc(it,SI);
                return true;
            }
            return false;
        };
        // 在溯源指令I的过程中，遇到指令CI和SI时，要判断它们的源是否控
        std::queue<GetElementPtrInst*> wait2analyze;
        for(auto it=I.use_begin();it!=I.use_end();it++){
            StoreInst* SI=dyn_cast<StoreInst>(it->getUser());
            CallInst* CI=dyn_cast<CallInst>(it->getUser());
            GetElementPtrInst* GEPI=dyn_cast<GetElementPtrInst>(it->getUser());
            if(GEPI&&GEPI->getOperand(0)==&I){
                wait2analyze.push(GEPI);
            }
            else{
                if(checkCISI(&I,it,SI,CI)){
                    break;
                }
            }
        }
        // 对于GEPI指令，一般是遇到了buffer[]或结构体、甚至是结构体数组，要不断解析GEPI指令，直到找到真正的源
        while(!wait2analyze.empty()){
            auto x=wait2analyze.front();
            wait2analyze.pop();
            for(auto it=x->use_begin();it!=x->use_end();it++){
                StoreInst* SI=dyn_cast<StoreInst>(it->getUser());
                CallInst* CI=dyn_cast<CallInst>(it->getUser());
                /*if(CI){
                    Global::logger->log("lm\t"+getString(*CI));
                }*/
                GetElementPtrInst* GEPI=dyn_cast<GetElementPtrInst>(it->getUser());
                if(GEPI&&GEPI->getOperand(0)==x){
                    wait2analyze.push(GEPI);
                }
                else{
                    if(checkCISI(x,it,SI,CI)){
                        break;
                    }
                }
            }
        }
    }
}

void BackTrace::visitGetElementPtrInst(GetElementPtrInst &I){
    Global::logger->log("visitGetElementPtrInst:\t" + getString(I) + "\n");
    if(this->is_arithmetic){
        //算数运算，将成员标记为源
        if(this->op_type==OperandType::SRC){
            this->gadget.src[&I]=this->pre_inst;
        }
        else{
            this->gadget.dst[&I]=this->pre_inst;
        }
    }
    for(int i=0;i<I.getNumOperands();i++){
        this->wait2visit.push({I.getOperand(i),this->op_type});
    }
}

void BackTrace::visitSelectInst(SelectInst &I){
    Global::logger->log("visitSelectInst[PASS]:\t"+ getString(I) + "\n");
}

void BackTrace::visitPHINode(PHINode &I){
    Global::logger->log("visitPHINode[PASS]:\t" + getString(I) + "\n");
}

void BackTrace::visitBitCastInst(BitCastInst &I){
    Global::logger->log("visitBitCastInst:\t" + getString(I) + "\n");
    this->wait2visit.push({I.getOperand(0),this->op_type});
}

void BackTrace::visitTruncInst(TruncInst &I){
    Global::logger->log("visitTruncInst:\t" + getString(I) + "\n");
    this->wait2visit.push({I.getOperand(0),this->op_type});
}

void BackTrace::visitSExtInst(SExtInst &I){
    Global::logger->log("visitSExtInst:\t" + getString(I) + "\n");
    this->wait2visit.push({I.getOperand(0),this->op_type});
}

void BackTrace::visitPtrToIntInst(PtrToIntInst &I){
    Global::logger->log("visitPtrToIntInst:\t" + getString(I) + "\n");
    this->wait2visit.push({I.getOperand(0),this->op_type});
}

void BackTrace::visitFPToSIInst(FPToSIInst &I){
    Global::logger->log("visitFPToSIInst:\t" + getString(I) + "\n");
    this->wait2visit.push({I.getOperand(0),this->op_type});
}

void BackTrace::visitBinaryOperator(BinaryOperator &I){
    Global::logger->log("visitBinaryOperator:\t" + getString(I) + "\n");
    //运算指令
    if(I.getOpcode()==Instruction::Add||I.getOpcode()==Instruction::Sub||I.getOpcode()==Instruction::Mul||I.getOpcode()==Instruction::URem||I.getOpcode()==Instruction::SRem){
        this->is_arithmetic=true;
    }
    this->wait2visit.push({I.getOperand(0),this->op_type});
    this->wait2visit.push({I.getOperand(1),this->op_type});
}

void BackTrace::visitInsertElementInst(InsertElementInst &I){
    Global::logger->log("visitInsertElementInst:\t" + getString(I) + "\n");
    exit(-1);
}

void BackTrace::visitICmpInst(ICmpInst &I){
    Global::logger->log("visitICmpInst:\t" + getString(I) + "\n");
    this->wait2visit.push({I.getOperand(0),this->op_type});
    this->wait2visit.push({I.getOperand(1),this->op_type});
}

void BackTrace::visitCallInst(CallInst &I){
    Global::logger->log("visitCallInst:\t" + getString(I) + "\n");
    //遇到复杂的函数就不再溯源
    this->has_a_call=true;
}