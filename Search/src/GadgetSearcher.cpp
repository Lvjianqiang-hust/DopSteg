#include "GadgetSearcher.h"

cl::opt<std::string> asdlfile("asdl",cl::init(""),cl::desc("semantic description language"));
cl::opt<std::string> overflowfunc("location",cl::init(""),cl::desc("location of overflow func"));
cl::opt<std::string> overflowvar("buffer",cl::init(""),cl::desc("buffer would be overflowed"));
cl::opt<std::string> varsize("buffersize",cl::init(""),cl::desc("buffer size"));
cl::opt<std::string> varname("buffername",cl::init(""),cl::desc("buffer name"));
cl::opt<std::string> overflowstatement("statement",cl::init(""),cl::desc("strcpy func"));
cl::opt<std::string> outfile("outfile",cl::init("./gadget"),cl::desc("result of search"));
cl::opt<std::string> locfile("locfile",cl::init("./location"),cl::desc("location info of overflow"));
cl::opt<std::string> logfile("logfile",cl::init("./log"),cl::desc("log file"));
cl::opt<std::string> dispatcher("dispatcher",cl::init(""),cl::desc("a function which has a dispatcher"));
cl::opt<std::string> range("range",cl::init(""),cl::desc("ALL or null"));

void GadgetSearcher::printGadgetInfo(Module &M){
    std::error_code EC;
    raw_fd_ostream f(outfile,EC,sys::fs::FA_Write);
    if(EC){
        Global::logger->log("Could not open file: " + EC.message());
        exit(-1);       
    }
    int num=0;
    auto left = M.getName().find_last_of('/');
    std::string md_name = M.getName().substr(left==std::string::npos?0:left+1,M.getName().size()-left-1);
    for(auto& x:this->gadgets){
        if(x.isGadget()){
            f<<num+1<<"\n";
            f<<"MODULE:\t"<<md_name<<"\n";
            f<<x<<"\n\n";
            num++;
        }
    }
    Global::logger->log("[check]\t" + std::string("Total gadgets(after filter):\t")+ std::to_string(num) + "\n");
    Global::logger->log("DONE!\n");
    f.close();
}

void GadgetSearcher::printLocationInfo(Module &M){
    auto vec = this->location_info.getLocationInfo();
    if(vec.size()==0){
        return;
    }
    std::error_code EC;
    raw_fd_ostream f(locfile,EC,sys::fs::FA_Write);
    if(EC){
        Global::logger->log("Could not open file: " + EC.message());
        exit(-1);       
    }
    auto left = M.getName().find_last_of('/');
    std::string md_name = M.getName().substr(left==std::string::npos?0:left+1,M.getName().size()-left-1);
    f<<"MODULE:\t"<<md_name<<"\n";
    for(auto& x:vec){
        f<<x<<"\n";
    }
}

bool GadgetSearcher::runOnModule(Module &M){
    //检查参数
    if(asdlfile==""||overflowfunc==""||overflowvar==""||varsize==""||varname==""||overflowstatement==""||dispatcher==""){
        Global::logger->log("Please inspect args!\n");
        exit(-1);
    }
    Global::logger = std::make_unique<Logger>(logfile,ILogger::Level::DEBUG);
    //根据描述语言分析需要的gadget类型
    Interpreter analyzer(asdlfile);
    analyzer.analyzeASDL(Interpreter::LEVEL::MODIFIABLE);
    GadgetInfo::setNeededGadget(analyzer.getNeededType());
    //获取漏洞函数信息
    this->vuln_func=overflowfunc;
    location_info.setBufferDeclaration(overflowvar);
    location_info.setBufferSize(varsize);
    location_info.setBufferName(varname);
    location_info.setStatement(overflowstatement);
    CallGraph CG(M);

    std::unordered_map<Function*,BackTrace::FuncType> f2type;
    auto start_f = M.getFunction(dispatcher); 
    auto vuln_fptr = M.getFunction(this->vuln_func);
    f2type[vuln_fptr]=BackTrace::FuncType::OVERFLOWFUNC;
    location_info.setFunctionPtr(vuln_fptr);
    CallGraphNode* CN = CG[vuln_fptr];
    for(int x=0;x<CN->size();x++){
        auto f = (*CN)[x]->getFunction();
        if(f&&f2type.count(f)==0){
            f2type[f] = BackTrace::FuncType::CHILD;
        }
    }
    int total = 0;
    for(auto& F:M){//有些函数只能通过函数指针调用
        if(F.isDeclaration()){
            Global::logger->log(F.getName().str() + " is declaration\n");
            continue;
        }
        if(start_f!=&F&&range!="ALL"){
            //不分析所有函数
            continue;
        }
        std::stack<FuncWithType> wait_visit;
        wait_visit.push({&F,f2type.count(&F)>0?f2type[&F]:BackTrace::FuncType::OTHER});

        while(!wait_visit.empty()){ //根据CG来遍历函数
            FuncWithType now_func=wait_visit.top();
            CallGraphNode *CN = CG[now_func.first];
            wait_visit.pop();
            if(this->visited.find(now_func.first)!=this->visited.end()){
                Global::logger->log(now_func.first->getName().str() + " has been visited\n");
                continue;
            }
            //add callee to queue
            int ignore{false};
            if(now_func.second==BackTrace::FuncType::OVERFLOWFUNC){
                //遇到漏洞函数，先分析它调用的函数
                wait_visit.emplace(now_func.first, now_func.second);
                ignore = true;
            }
            for (unsigned int x=0; x<CN->size(); x++){
                Function *func=(*CN)[x]->getFunction();
                
                if (func!=nullptr){
                    if(func->isDeclaration()){
                        Global::logger->log(func->getName().str() + " is declaration\n");
                    }
                    else if(this->visited.find(func)!=this->visited.end()){
                        Global::logger->log(func->getName().str() + " has been visited\n");
                    }
                    else{
                        Global::logger->log(func->getName().str() + " has been push\n");
                        BackTrace::FuncType ftype;
                        if(f2type.count(func)){
                            ftype=f2type[func];
                        }
                        else{
                            ftype=BackTrace::FuncType::OTHER;
                        }
                        
                        wait_visit.push({func,ftype});
                    }
                }
                else{
                    Global::logger->log("unknown (*CN)[x]\n");
                }
            }
            if(ignore&&wait_visit.top().first!=now_func.first){
                continue;
            }

            Global::logger->log("visit " + now_func.first->getName().str() + "\n");
            this->visited.insert(now_func.first);

            BackTrace::interesting_storeinst.clear();  //只有在同一个函数中，记录的信息才有用
            int count=0;
            for(auto& BB:*now_func.first){
                for(auto& I:BB){
                    if(StoreInst* SI=dyn_cast<StoreInst>(&I)){
                        Global::logger->log("[check]\t"+getString(*SI)+"\n");
                        //errs() << "[check]\t"+getString(*SI)+"\n";
                        //分析store指令
                        BackTrace bt{SI,now_func.second,false};
                        if(bt.isInteresting()){
                            total++;
                            this->gadgets.push_back(GadgetInfo(bt.getGadget(),now_func.first,count,false));
                            Global::logger->log("FOUND Gadget:\t"+getString(*SI)+"\n");
                        }
                        Global::logger->log("\n\n");
                    }
                    else if(CallInst* CI=dyn_cast<CallInst>(&I)){
                        Global::logger->log("[check]\t" + getString(*CI) + "\n");
                        //errs() << "[check]\t"+getString(*CI)+"\n";
                        //分析print和copy函数
                        if(CI->getCalledFunction()&&(Global::print_func.find(CI->getCalledFunction()->getName())!=Global::print_func.end()||
                        Global::copy_func.find(CI->getCalledFunction()->getName())!=Global::copy_func.end())){
                            BackTrace bt{CI,now_func.second,true};
                            if(bt.isInteresting()){
                                total++;    
                                this->gadgets.push_back(GadgetInfo(bt.getGadget(),now_func.first,count,true));
                                Global::logger->log("FOUND Gadget:\t" + getString(*CI) + "\n");
                            }
                            Global::logger->log("\n\n");
                        }
                    }
                    count++;
                }
            }  
        }
    }
    Global::logger->log("[check]\t" + std::string("Total gadgets(before filter):\t")+ std::to_string(total) + "\n");
    this->printGadgetInfo(M);
    this->printLocationInfo(M);
    return false;
}

char GadgetSearcher::ID = 0;

static RegisterPass<GadgetSearcher> X("search","Find all gadgets!");
