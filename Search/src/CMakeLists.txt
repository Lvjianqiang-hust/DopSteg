project(search)

SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++") # 填写绝对路径
SET (CMAKE_CXX_FLAGS                "-std=c++17 -g -fno-limit-debug-info -Wno-unknown-warning-option")

add_library(GadgetSearcher SHARED Interpreter.h Interpreter.cpp Utils.h Utils.cpp LocationInfo.h LocationInfo.cpp GadgetInfo.h GadgetInfo.cpp BackTrace.h BackTrace.cpp GadgetSearcher.h GadgetSearcher.cpp)