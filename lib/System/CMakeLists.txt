file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.inc")

#set(LLVMSystem ${SRCS} CACHE INTERNAL "")
add_library(LLVMSystem STATIC ${SRCS})
