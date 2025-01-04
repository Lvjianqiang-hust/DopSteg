#include "Utils.h"

std::set<std::string> Global::copy_func{"llvm.memcpy.p0i8.p0i8.i32","strcpy","strncpy","vsnprintf","snprintf"};
std::set<std::string> Global::print_func{"putc","printf","fputc","fprintf","send","write","sendmsg"};
std::unique_ptr<ILogger> Global::logger;

std::string llvm::getString(){
    return "";
}

std::string ILogger::getLevelStr(Level _level){
    switch(_level){
        case Level::DEBUG:
            return "[DEBUG]";
        case Level::ERR:
            return "[ERR]";
        default:
            return "";
    }
}

Logger::Logger(const std::string& _file, Level _level) : out(_file,EC,sys::fs::FA_Write), level(_level){
    if(EC){
        errs() << "Could not open file: " << EC.message();
        exit(-1);       
    }
}

Logger::~Logger(){
    out.close();
}

void Logger::log(const std::string& _str){
    out << "[" << getLevelStr(this->level) << "]\t" << _str << "\n";
}

void Logger::setLevel(Level _level){
    this->level = _level;
}