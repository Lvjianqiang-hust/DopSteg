#include"Interpreter.h"

Interpreter::Interpreter(const std::string& file):filename(file),needed_type(std::vector<std::string>{"RWA"}),is_needed_dispathcer(false){
}

void Interpreter::analyzeASDL(LEVEL level){
    if(level==LEVEL::NONMODIFIABLE){
        this->needed_type.push_back("ASSIGNMENT");
    }
    std::ifstream in(this->filename);
    if(!in.is_open()){
        exit(-1); 
    }
    char buffer[1024];
    while(!in.eof()){
        in.getline(buffer, 1024);
        std::string buf(buffer);
        if(buf.find("void ")!=std::string::npos){
            continue;
        }
        if(buf.find("for ")!=std::string::npos){
            is_needed_dispathcer=true;
        }
        else if(buf.find("*")!=std::string::npos){
            if(buf.find(" * ")!=std::string::npos){
                this->needed_type.push_back("MUL");
            }
            else{//遇到了解引用
                this->needed_type.push_back("DEREF");
            }
        }
        else if(buf.find(" + ")!=std::string::npos){
            this->needed_type.push_back("ADD");
        }
        else if(buf.find(" - ")!=std::string::npos){
            this->needed_type.push_back("SUB");
        }
        else if(buf.find(" ^ ")!=std::string::npos){
            this->needed_type.push_back("XOR");
        }
        else if(buf.find(" % ")!=std::string::npos){
            this->needed_type.push_back("REM");
        }
        else if(buf.find("print(")!=std::string::npos){
            this->needed_type.push_back("OUT");
        }
        else if(buf.find("SYS ")!=std::string::npos){
            std::size_t left = buf.find("SYS ") + 4;
            std::size_t right = buf.find("(",left);
            this->needed_SYS.push_back(buf.substr(left,right-left));
        }
        else if(buf.find(" = ")!=std::string::npos){
            this->needed_type.push_back("ASSIGNMENT");
        }
    }
    in.close();
}