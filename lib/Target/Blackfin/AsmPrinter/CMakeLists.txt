include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMBlackfinAsmPrinter
        BlackfinAsmPrinter.cpp
  )
add_dependencies(LLVMBlackfinAsmPrinter BlackfinCodeGenTable_gen)
