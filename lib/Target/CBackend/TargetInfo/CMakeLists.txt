include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ..)

add_llvm_library(LLVMCBackendInfo
        CBackendTargetInfo.cpp
  )

