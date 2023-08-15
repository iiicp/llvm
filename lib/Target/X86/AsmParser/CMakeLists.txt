include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMX86AsmParser
        X86AsmParser.cpp
  )
add_dependencies(LLVMX86AsmParser X86CodeGenTable_gen)
