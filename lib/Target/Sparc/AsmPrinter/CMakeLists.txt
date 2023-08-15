include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMSparcAsmPrinter
        SparcAsmPrinter.cpp
  )
add_dependencies(LLVMSparcAsmPrinter SparcCodeGenTable_gen)