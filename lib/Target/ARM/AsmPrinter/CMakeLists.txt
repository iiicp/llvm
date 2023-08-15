include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMARMAsmPrinter
        ARMAsmPrinter.cpp
  )
add_dependencies(LLVMARMAsmPrinter ARMCodeGenTable_gen)