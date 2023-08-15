include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}/..
        ..
  )

add_llvm_library(LLVMMipsAsmPrinter
        MipsAsmPrinter.cpp
  )
add_dependencies(LLVMMipsAsmPrinter MipsCodeGenTable_gen)