include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMXCoreInfo
        XCoreTargetInfo.cpp
  )

add_dependencies(LLVMXCoreInfo XCoreTable_gen)
