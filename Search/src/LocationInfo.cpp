#include "LocationInfo.h"
std::vector<std::string> LocationInfo::getLocationInfo(){
    std::vector<std::string> ans;
    if(this->buffer_declaration==""||this->buffer_name==""||this->buffer_size==""||this->statement==""||this->F==nullptr){
        return ans;
    }
    int right_bound = this->statement.find_last_of(',');
    int state_index = stoi(statement.substr(right_bound+1, statement.size()-right_bound-1));
    std::string call_func = statement.substr(0,right_bound);
    ans.push_back("FUNC:\t"+std::string(this->F->getName()));
    int now_index = 0;
    int count=0;
    for(auto& BB:*this->F){
        for(auto& I:BB){
            AllocaInst* alloca = dyn_cast<AllocaInst>(&I);
            if(alloca){
                std::string alloca_str = getString(*alloca);
                int loc1 = alloca_str.find_first_of('%');
                int loc2 = alloca_str.find_first_of(' ',loc1);
                std::string var_name = alloca_str.substr(loc1+1, loc2-loc1-1);
                if(var_name == this->buffer_declaration){
                    ans.push_back("BUF:\t"+alloca_str);
                    ans.push_back("BUFINDEX:\t"+std::to_string(count));
                    ans.push_back("BUFNAME:\t"+this->buffer_name);
                    ans.push_back("BUFSIZE:\t"+this->buffer_size);
                }
            }
            CallInst* call = dyn_cast<CallInst>(&I);
            if(call){
                std::string func_str = call->getCalledFunction()->getName();
                if(func_str==call_func){
                    now_index++;
                    if(now_index==state_index){
                        ans.push_back("STMT:\t"+getString(*call));
                        ans.push_back("STMTINDEX:\t"+std::to_string(count));
                        return ans;
                    }
                }
            }
            count++; 
        }
    }

    return ans;
}
void LocationInfo::setFunctionPtr(Function* func){
    this->F=func;
}
void LocationInfo::setBufferDeclaration(const std::string& buf_declaration){
    this->buffer_declaration=buf_declaration;
}
void LocationInfo::setBufferName(const std::string& buf_name){
    this->buffer_name=buf_name;
}
void LocationInfo::setBufferSize(const std::string& buf_size){
    this->buffer_size=buf_size;
}
void LocationInfo::setStatement(const std::string& state){
    this->statement=state;
}