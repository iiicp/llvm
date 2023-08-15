include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMARMInfo
        ARMTargetInfo.cpp
  )

add_dependencies(LLVMARMInfo ARMCodeGenTable_gen)
