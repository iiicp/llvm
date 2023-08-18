file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
#set(LLVMSupport ${SRCS} CACHE INTERNAL "")
add_library(LLVMSupport STATIC ${SRCS})
target_link_libraries(LLVMSupport PRIVATE LLVMSystem)