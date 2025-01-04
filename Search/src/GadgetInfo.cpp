#include "GadgetInfo.h"

std::map<Value*,std::pair<bool,int>> GadgetInfo::val2info;
std::map<std::pair<Value*,std::string>,std::pair<bool,int>> GadgetInfo::struct2info;

GadgetInfo::GadgetInfo(const Gadget& g,Function* f,int id,bool iscall):isgadget(true),gadget(g),gadget_type(GadgetType::NONE),func(f),index(id){
    this->analyzeGadget(iscall);
}

void GadgetInfo::analyzeGadget(bool iscall){
    auto checkSource = [](decltype(this->gadget.src)& source){
        for(auto&& [src,loc]:source){
            auto type_str = getString(*src->getType());
            if(type_str.substr(0,8)=="%struct."&&(type_str.substr(type_str.size()-2,2)=="**"||type_str.back()=='*'&&dyn_cast<Argument>(src))){
                return true;
            }
        }
        return false;
    };

    auto checkDerefType = [this](decltype(this->gadget.src)& source){
        //如果src是**local_p或*global_p
        for(auto& x:source){
            if(getString(*(x.first)->getType()).find("**")!=std::string::npos&&dyn_cast<GlobalVariable>(x.first)
            ||getString(*(x.first)->getType()).find("***")!=std::string::npos&&dyn_cast<AllocaInst>(x.first)){
                this->gadget_type=GadgetType::DEREF;
                break;
            }
        }
        if(this->gadget_type==GadgetType::DEREF){
            //store指令的源必须是value，不能是addr
            StoreInst* s = dyn_cast<StoreInst>(this->gadget.inst);
            if(s&&getString(*s->getOperand(0)->getType()).find('*')!=std::string::npos){
                this->gadget_type=GadgetType::NONE;
            }
        }
    };

    if(checkSource(this->gadget.src)||checkSource(this->gadget.dst)){
        this->isgadget=false;
        return;
    }

    if(iscall){
        CallInst* CI=dyn_cast<CallInst>(this->gadget.inst);
        std::string func_name=CI->getCalledFunction()->getName();
        if(Global::print_func.count(func_name)){
            //如果源中没有char*类型的，不是OUT类型gadget
            bool is_buffer{false};
            for(auto&& [src,loc]:this->gadget.src){
                auto type_str = getString(*src->getType());
                if(type_str=="i8**"||type_str=="i8*"&&dyn_cast<Argument>(src)||type_str.find("x i8]*")!=std::string::npos){
                    is_buffer = true;
                    break;
                }
            }
            if(!is_buffer){
                this->isgadget=false;
                return;
            }

            //不需要fprintf(stderr,)
            if(this->isgadget&&func_name=="fprintf"){
                LoadInst* LI=dyn_cast<LoadInst>(CI->getArgOperand(0));
                if(LI){
                    auto str=getString(*LI);
                    if(str.find("@stderr")!=std::string::npos){
                        this->isgadget=false;
                    }
                }
            }
            
            this->gadget_type=GadgetType::OUT;
        }
        else if(Global::copy_func.count(func_name)){
            bool is_RA{false};
            for(auto&& [src,loc]:this->gadget.src){
                auto type_str = getString(*src->getType());
                bool is_arg_src = type_str.find("*")!=std::string::npos&&dyn_cast<Argument>(src);
                if(type_str.find("**")!=std::string::npos || type_str.find("x i8*]*")!=std::string::npos || is_arg_src){
                    is_RA = true;
                    break;
                }
            }

            bool is_WA{false};
            for(auto&& [dst,loc]:this->gadget.dst){
                auto type_str = getString(*dst->getType());
                bool is_arg_dst = type_str.find("*")!=std::string::npos&&dyn_cast<Argument>(dst);
                if(type_str.find("**")!=std::string::npos || type_str.find("x i8*]*")!=std::string::npos || is_arg_dst){
                    is_WA = true;
                    break;
                }
            }
            if(is_RA&&is_WA&&func_name!="vsnprintf"&&func_name!="snprintf"){
                this->gadget_type=GadgetType::RWA;
                checkDerefType(this->gadget.src);
            }

            if(this->gadget_type==GadgetType::NONE){
                //必须有一个是全局变量，一个是局部变量
                if(this->gadget.src.size()==1&&this->gadget.dst.size()==1&&
                    ((dyn_cast<GlobalVariable>(this->gadget.src.begin()->first)&&!dyn_cast<GlobalVariable>(this->gadget.dst.begin()->first)) ||
                     (!dyn_cast<GlobalVariable>(this->gadget.src.begin()->first)&&dyn_cast<GlobalVariable>(this->gadget.dst.begin()->first))
                    )){
                    this->gadget_type=GadgetType::ASSIGNMENT;
                }
                else{
                    this->isgadget=false;
                }
            }
        }
    }
    else{
        StoreInst* SI=dyn_cast<StoreInst>(this->gadget.inst);
        Value* src_value=SI->getValueOperand();
        Value* dst_value=SI->getPointerOperand();

        //判断类型并筛选   
        GadgetType pre_type=GadgetType::NONE;
        std::vector<GadgetType> types;  //涉及的运算类型
        for(auto& ins_ptr:this->gadget.src_inst){
            //确认是否是算数运算
            switch (ins_ptr->getOpcode()){
                case Instruction::Xor:
                    types.insert(types.begin(), GadgetType::XOR);
                    break;
                case Instruction::Add:
                    types.insert(types.begin(), GadgetType::ADD);
                    break;
                case Instruction::Sub:
                    types.insert(types.begin(), GadgetType::SUB);
                    break;                    
                case Instruction::Mul:
                    types.insert(types.begin(), GadgetType::MUL);
                    break;                    
                case Instruction::URem:
                case Instruction::SRem:
                    types.insert(types.begin(), GadgetType::REM);
                    break;                    
                default:
                    break;
            }
        }
        //需要所含有的运算类型都是需要的，先不考虑需要的顺序问题
        for(auto& x:types){
            if(needed_gadget.find(x)==needed_gadget.end()){
                //发现了一个不需要的运算类型
                this->isgadget=false;
                break;
            }
        }
        
        if(this->isgadget){

            //运算Gadget？
            if(types.size()==1){
                this->gadget_type=types.back();
            }
            else if(types.size()>1){
                this->gadget_type=GadgetType::ARITHMETIC;
            }

            //DEREF？
            if(this->gadget_type==GadgetType::NONE){
                //不是算数运算，则判断是不是解引用      
                checkDerefType(this->gadget.src);
            }

            //RWA？
            if(this->gadget_type==GadgetType::NONE){
                //不是算数运算，也不是解引用，则判断是不是任意地址读/写
                bool is_RA=false;
                bool is_arg_src{false};
                for(auto& x:this->gadget.src){
                    is_arg_src = getString(*(x.first)->getType()).find("*")!=std::string::npos&&dyn_cast<Argument>(x.first);
                    if((getString(*(x.first)->getType()).find("**")!=std::string::npos || is_arg_src)
                    &&getString(*dyn_cast<StoreInst>(this->gadget.inst)->getOperand(0)->getType()).find('*')==std::string::npos){
                        //src中要有指针，且store的src不能是addr
                        is_RA=true; 
                        break;
                    }
                }
                bool is_WA=false;
                bool is_arg_dst{false};
                for(auto& x:this->gadget.dst){
                    is_arg_dst = getString(*(x.first)->getType()).find("*")!=std::string::npos&&dyn_cast<Argument>(x.first);
                    if((getString(*(x.first)->getType()).find("**")!=std::string::npos || is_arg_dst)
                    &&getString(*dyn_cast<StoreInst>(this->gadget.inst)->getPointerOperandType()).find("**")==std::string::npos){
                        //dst中要有指针，且store的dst只能是一级地址
                        is_WA=true; 
                        break;
                    }
                }
                if(is_RA&&is_WA){
                    this->gadget_type=GadgetType::RWA;
                    if(is_arg_src&&is_arg_dst){
                        //将sstrncpy设置为copy函数
                        Global::copy_func.insert(this->func->getName());
                    }
                }
            }

            //ASSIGNMENT？
            if(this->gadget_type==GadgetType::NONE){
                //必须有一个是全局变量，一个是局部变量
                if(this->gadget.src.size()==1&&this->gadget.dst.size()==1&&
                    ((dyn_cast<GlobalVariable>(this->gadget.src.begin()->first)&&!dyn_cast<GlobalVariable>(this->gadget.dst.begin()->first)) ||
                     (!dyn_cast<GlobalVariable>(this->gadget.src.begin()->first)&&dyn_cast<GlobalVariable>(this->gadget.dst.begin()->first))
                    )){
                    this->gadget_type=GadgetType::ASSIGNMENT;
                }
            }
        }

        if(this->isgadget&&needed_gadget.find(this->gadget_type)==needed_gadget.end()){
            //虽然是gadget，但不需要
            this->isgadget=false;
        }
    }
    if(!this->isgadget||this->gadget_type==GadgetType::NONE){
        this->isgadget=false;
        return;
    }
    //计算this->gadget.src、this->gadget.dst中的各个源的使用频率
    //判断它们是否在getelementptr指令中作为index来使用

    auto isIndex = [this](Value* v){
        for(auto tmp_it=v->use_begin();tmp_it!=v->use_end();tmp_it++){
            if(GetElementPtrInst* GEPI=dyn_cast<GetElementPtrInst>(tmp_it->getUser())){
                return true;
            }
        }
        return false;
    };

    auto setCtWithFlag = [this,isIndex](Value* var,std::string key,int& ct,bool& flag_index){
        for(auto it=var->use_begin();it!=var->use_end();it++){
            if(GetElementPtrInst* g=dyn_cast<GetElementPtrInst>(it->getUser())){
                auto str = this->getGEPIKey(g);
                if(str==key){
                    ct++;
                    flag_index = flag_index || isIndex(g);
                }
            }
            else if(ConstantExpr* c=dyn_cast<ConstantExpr>(it->getUser())){
                auto str = this->getGEPIKey(c);
                if(str==key){
                    ct++;
                    flag_index = flag_index || isIndex(c);
                }
            }
        }
    };

    auto checkVar = [this,isIndex,setCtWithFlag](decltype(this->gadget.src)& source){
        bool flag_res{false};   //source是否存在一个源：它是索引？
        int ct_sum{0};          //所有源的使用次数之和
        for(auto& [var,ins]:source){
            auto type_str = getString(*var->getType()); //用于判断是不是struct类型
            bool flag_index{false}; //当前源是否是索引
            int ct{0};  //当前源的使用次数之和
            if(type_str.find("%struct.")!=std::string::npos){
                std::string key;
                //首先要获取key string
                if(LoadInst* LI=dyn_cast<LoadInst>(ins)){
                    if(ConstantExpr *CE = dyn_cast<ConstantExpr>(LI->getPointerOperand())){
                        if(CE->getOpcode()==Instruction::GetElementPtr){
                            key = this->getGEPIKey(CE);
                        }
                        else{//遇到了未知情况，则不认为是gadget
                            this->isgadget=false;
                            return std::pair{false,2};                            
                        }
                    }
                    else{//遇到了未知情况，则不认为是gadget
                        this->isgadget=false;
                        return std::pair{false,2};
                    }
                }
                else if(GetElementPtrInst* GEPI=dyn_cast<GetElementPtrInst>(ins)){
                    key = this->getGEPIKey(GEPI);
                }
                else{//遇到了未知情况，则不认为是gadget
                    this->isgadget=false;
                    return std::pair{false,2}; 
                }

                if(GadgetInfo::struct2info.count({var,key})){
                    //已经计算过就直接使用之前的结果
                    flag_index = GadgetInfo::struct2info[{var,key}].first;
                    ct = GadgetInfo::struct2info[{var,key}].second;
                }
                else{
                    setCtWithFlag(var,key,ct,flag_index);                
                    GadgetInfo::struct2info[std::pair{var,key}]={flag_index,ct};
                }
            }
            else{
                if(GadgetInfo::val2info.count(var)){
                    //已经计算过就直接使用之前的结果
                    flag_index = GadgetInfo::val2info[var].first;
                    ct = GadgetInfo::val2info[var].second;
                }
                else{
                    for(auto it=var->use_begin();it!=var->use_end();it++){
                        if(LoadInst* LI=dyn_cast<LoadInst>(it->getUser())){
                            //判断LI是否在getelementptr中
                            flag_index = flag_index || isIndex(LI);
                            if(flag_index){
                                break;
                            }
                        }
                    }
                    ct = var->getNumUses();
                    GadgetInfo::val2info[var]={flag_index,ct};
                }
            }
            ct_sum += ct;
            flag_res = flag_res || flag_index;
        }
        return std::pair{flag_res,ct_sum};
    };
    
    auto [flag_src,ct_src] = checkVar(this->gadget.src);
    auto [flag_dst,ct_dst] = checkVar(this->gadget.dst);
    this->gadget.isindex = flag_src || flag_dst;
    this->gadget.use_num = ct_src + ct_dst;
}

raw_ostream& llvm::operator << (raw_ostream& out,GadgetInfo& g){
    std::string type;
    switch(g.getType()){
        case GadgetInfo::GadgetType::ASSIGNMENT:
            type="ASSIGNMENT";
            break;
        case GadgetInfo::GadgetType::DEREF:
            type="DEREF";
            break;
        case GadgetInfo::GadgetType::ARITHMETIC:
            type="ARITHMETIC";
            break;
        case GadgetInfo::GadgetType::OUT:
            type="OUT";
            break; 
        case GadgetInfo::GadgetType::RWA:
            type="RWA";
            break; 
        case GadgetInfo::GadgetType::ADD:
            type="ADD";
            break;
        case GadgetInfo::GadgetType::SUB:
            type="SUB";
            break;
        case GadgetInfo::GadgetType::MUL:
            type="MUL";
            break;
        case GadgetInfo::GadgetType::REM:
            type="REM";
            break;
        case GadgetInfo::GadgetType::XOR:
            type="XOR";
            break;        
        default:
            type="NONE";
            break;
    }

    auto put_type_name=[&out](std::string type,Value* v,Instruction* ins){
        std::stringstream s(getString(*v));
        std::string sptr;
        s>>sptr;    //去掉空格前缀
        if(sptr.find('@')!=std::string::npos||sptr.find('%')!=std::string::npos){
            //全局或局部变量
            std::string type_str=getString(*v->getType());
            out<<type<<"("<<type_str.substr(0,type_str.size()-1)<<" "<<sptr<<")";
        }
        else{
            //参数变量
            out<<type<<"("<<*v<<")";
        }
        if(ins){
            out<<*ins<<"\n";
        }
        else{
            out<<"\n";
        }
    };

    out<<"FUNC:\t"<<g.func->getName()<<"\n";
    out<<"INDEX:\t"<<g.index<<"\n";
    out<<"TYPE:\t"<<type<<"\n";
    for(auto v:g.gadget.src){
        put_type_name("SRC:\t",v.first,v.second);
    }
    for(auto& ins_ptr:g.gadget.src_inst){
        out<<"SRC_BACKTRACE:"<<*ins_ptr<<"\n";
    }
    for(auto v:g.gadget.dst){
        put_type_name("DST:\t",v.first,v.second);
    }
    for(auto& ins_ptr:g.gadget.dst_inst){
        out<<"DST_BACKTRACE:"<<*ins_ptr<<"\n";
    }
    out<<"STORE:"<<*g.gadget.inst<<"\n";

    int ct{0};
    for(auto it = g.func->use_begin();it!=g.func->use_end();it++){
        if(CallInst* call=dyn_cast<CallInst>(it->getUser())){
            ct++;
        }
    }
    out<<"FCALL_NUM:\t"<<ct<<"\n";
    
    out<<"ISINDEX:\t"<<g.gadget.isindex<<"\n";
    out<<"COUNT:\t"<<g.gadget.use_num<<"\n";
    return out;
}

std::string GadgetInfo::getGEPIKey(GetElementPtrInst* GEPI){
    std::string str(getString(*GEPI));
    //避免因为嵌套struct导致%struct.在getelementptr前出现
    int left = str.find("getelementptr");
    left = str.find("%struct.",left+std::string("getelementptr").size());
    int right = str.rfind(", !dbg");
    return str.substr(left,right-left);
}
std::string GadgetInfo::getGEPIKey(ConstantExpr* GEPI){
    std::string str(getString(*GEPI));
    //避免因为嵌套struct导致%struct.在getelementptr前出现
    int left = str.find("getelementptr");
    left = str.find("%struct.",left+std::string("getelementptr").size());
    int right = str.rfind(")");
    if(right==-1){
        return str.substr(left);
    }
    else{
        return str.substr(left,right-left);
    }
}

std::unordered_set<GadgetInfo::GadgetType> GadgetInfo::needed_gadget{
        //默认需要的类型
        GadgetInfo::GadgetType::DEREF,  
        GadgetInfo::GadgetType::ARITHMETIC,
        GadgetInfo::GadgetType::RWA,
        GadgetInfo::GadgetType::XOR,
        GadgetInfo::GadgetType::ADD,
        GadgetInfo::GadgetType::SUB,
        GadgetInfo::GadgetType::OUT,
        GadgetInfo::GadgetType::MUL,
        GadgetInfo::GadgetType::REM
};

std::unordered_map<std::string,GadgetInfo::GadgetType> GadgetInfo::s_to_gtype{
    {"ASSIGNMENT",GadgetInfo::GadgetType::ASSIGNMENT},  //根据是否可以插装目标程序来决定是否需要这两类gadget
    {"ARITHMETIC",GadgetInfo::GadgetType::ARITHMETIC},

    {"RWA",GadgetInfo::GadgetType::RWA},    //这种必须存在

    {"DEREF",GadgetInfo::GadgetType::DEREF},    //根据攻击需求来决定是否需要
    {"XOR",GadgetInfo::GadgetType::XOR},
    {"ADD",GadgetInfo::GadgetType::ADD},
    {"SUB",GadgetInfo::GadgetType::SUB},
    {"MUL",GadgetInfo::GadgetType::MUL},
    {"REM",GadgetInfo::GadgetType::REM},
    {"OUT",GadgetInfo::GadgetType::OUT}
};
