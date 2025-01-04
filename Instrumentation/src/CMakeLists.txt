project(Instrumentation)

SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++") # 填写绝对路径
SET (CMAKE_CXX_FLAGS                "-std=c++17 -g -fno-limit-debug-info -Wno-unknown-warning-option")

add_library( Instrumentation SHARED
  GadgetAttacher.cpp
  GadgetGenerator.cpp
  GadgetController.cpp
  LLVMGlueLayer.cpp
  )
