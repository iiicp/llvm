include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMMSP430AsmPrinter
        MSP430AsmPrinter.cpp
  )
add_dependencies(LLVMMSP430AsmPrinter MSP430CodeGenTable_gen)
