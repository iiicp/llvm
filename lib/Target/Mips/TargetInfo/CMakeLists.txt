include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMMipsInfo
        MipsTargetInfo.cpp
  )

add_dependencies(LLVMMipsInfo MipsCodeGenTable_gen)
