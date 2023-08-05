add_subdirectory(Support)
add_subdirectory(System)

message(STATUS "${LLVMSupport}")
message(STATUS "${LLVMSystem}")

set(llvm_src
    ${LLVMSupport}
    ${LLVMSystem})

add_library(${PROJECT_NAME} STATIC ${llvm_src})