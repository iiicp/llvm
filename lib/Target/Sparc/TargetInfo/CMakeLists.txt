include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMSparcInfo
        SparcTargetInfo.cpp
  )

add_dependencies(LLVMSparcInfo SparcCodeGenTable_gen)
