#pragma once
#include "Utils.h"

namespace llvm{
    class LocationInfo{
    public:
        std::vector<std::string> getLocationInfo();
        void setFunctionPtr(Function* func);
        void setBufferDeclaration(const std::string& buf_declaration);
        void setBufferName(const std::string& buf_name);
        void setBufferSize(const std::string& buf_size);
        void setStatement(const std::string& state);
    private:
        std::string buffer_declaration;
        std::string buffer_size;
        std::string buffer_name;
        std::string statement;
        Function* F{nullptr};
    };
}

