include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMMSP430Info
        MSP430TargetInfo.cpp
  )

add_dependencies(LLVMMSP430Info MSP430Table_gen)
