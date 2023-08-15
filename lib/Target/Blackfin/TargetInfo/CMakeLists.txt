include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMBlackfinInfo
        BlackfinTargetInfo.cpp
  )

add_dependencies(LLVMBlackfinInfo BlackfinCodeGenTable_gen)
