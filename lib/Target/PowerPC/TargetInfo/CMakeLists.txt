include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMPowerPCInfo
        PowerPCTargetInfo.cpp
  )

add_dependencies(LLVMPowerPCInfo PowerPCCodeGenTable_gen)
