include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
        ..
  )

add_llvm_library(LLVMCellSPUAsmPrinter
        SPUAsmPrinter.cpp
  )
add_dependencies(LLVMCellSPUAsmPrinter CellSPUCodeGenTable_gen)