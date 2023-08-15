include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMSystemZInfo
        SystemZTargetInfo.cpp
  )

add_dependencies(LLVMSystemZInfo SystemZCodeGenTable_gen)
