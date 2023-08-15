include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMCellSPUInfo
        CellSPUTargetInfo.cpp
  )

add_dependencies(LLVMCellSPUInfo CellSPUCodeGenTable_gen)
