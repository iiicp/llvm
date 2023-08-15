# See docs/CMake.html for instructions about how to build LLVM with CMake.
cmake_minimum_required(VERSION 3.18)
project(LLVM)

set(PACKAGE_NAME llvm)
set(PACKAGE_VERSION 2.6svn)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "llvmbugs@cs.uiuc.edu")

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed.")
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

set(LLVM_MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LLVM_MAIN_INCLUDE_DIR ${LLVM_MAIN_SRC_DIR}/include)
set(LLVM_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(LLVM_TOOLS_BINARY_DIR ${LLVM_BINARY_DIR}/bin)
set(LLVM_EXAMPLES_BINARY_DIR ${LLVM_BINARY_DIR}/examples)
set(LLVM_LIBDIR_SUFFIX "" CACHE STRING "Define suffix of library directory name (32/64)" )

include_directories(include)

set(LLVM_ALL_TARGETS
        Alpha
        ARM
        Blackfin
        CBackend
        CellSPU
        CppBackend
        Mips
        MSIL
        MSP430
        PIC16
        PowerPC
        Sparc
        SystemZ
        X86
        XCore
        )

set(LLVM_TARGETS_TO_BUILD X86)

option(LLVM_ENABLE_THREADS "Use threads if available." ON)

if( uppercase_CMAKE_BUILD_TYPE STREQUAL "RELEASE" )
    option(LLVM_ENABLE_ASSERTIONS "Enable assertions" OFF)
else()
    option(LLVM_ENABLE_ASSERTIONS "Enable assertions" ON)
endif()

if( LLVM_ENABLE_ASSERTIONS )
    # MSVC doesn't like _DEBUG on release builds. See PR 4379.
    if( NOT MSVC )
        add_definitions( -D_DEBUG )
    endif()
    # On Release builds cmake automatically defines NDEBUG, so we
    # explicitly undefine it:
    if( uppercase_CMAKE_BUILD_TYPE STREQUAL "RELEASE" )
        add_definitions( -UNDEBUG )
    endif()
else()
    if( NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "RELEASE" )
        add_definitions( -DNDEBUG )
    endif()
endif()

if( uppercase_CMAKE_BUILD_TYPE STREQUAL "RELEASE" AND NOT LLVM_ENABLE_ASSERTIONS )
    set( LLVM_COMPACT_SENTINELS 1 )
else( uppercase_CMAKE_BUILD_TYPE STREQUAL "RELEASE" AND NOT LLVM_ENABLE_ASSERTIONS )
    set( LLVM_COMPACT_SENTINELS 0 )
endif( uppercase_CMAKE_BUILD_TYPE STREQUAL "RELEASE" AND NOT LLVM_ENABLE_ASSERTIONS )


set(LLVM_ENUM_TARGETS "")
foreach(c ${LLVM_TARGETS_TO_BUILD})
    list(FIND LLVM_ALL_TARGETS ${c} idx)
    if( idx LESS 0 )
        message(FATAL_ERROR "The target `${c}' does not exist.
    It should be one of\n${LLVM_ALL_TARGETS}")
    else()
        set(LLVM_ENUM_TARGETS "${LLVM_ENUM_TARGETS}LLVM_TARGET(${c})\n")
    endif()
endforeach(c)

# Produce llvm/Config/Targets.def
configure_file(
        ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/Targets.def.in
        ${LLVM_BINARY_DIR}/include/llvm/Config/Targets.def
)

set(llvm_builded_incs_dir ${LLVM_BINARY_DIR}/include/llvm)

# Add path for custom modules
set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        "${LLVM_MAIN_SRC_DIR}/cmake"
        "${LLVM_MAIN_SRC_DIR}/cmake/modules"
        )


if(WIN32)
    if(CYGWIN)
        set(LLVM_ON_WIN32 0)
        set(LLVM_ON_UNIX 1)
    else(CYGWIN)
        set(LLVM_ON_WIN32 1)
        set(LLVM_ON_UNIX 0)
    endif(CYGWIN)
    set(LTDL_SHLIB_EXT ".dll")
    set(EXEEXT ".exe")
    # Maximum path length is 160 for non-unicode paths
    set(MAXPATHLEN 160)
else(WIN32)
    if(UNIX)
        set(LLVM_ON_WIN32 0)
        set(LLVM_ON_UNIX 1)
        set(LTDL_SHLIB_EXT ".so")
        set(EXEEXT "")
        # FIXME: Maximum path length is currently set to 'safe' fixed value
        set(MAXPATHLEN 2024)
    else(UNIX)
        MESSAGE(SEND_ERROR "Unable to determine platform")
    endif(UNIX)
endif(WIN32)

include(config-ix)
include(AddLLVMDefinitions)

option(LLVM_ENABLE_PIC "Build Position-Independent Code" ON)

set(ENABLE_PIC 0)
if( LLVM_ENABLE_PIC )
    if( SUPPORTS_FPIC_FLAG )
        message(STATUS "Building with -fPIC")
        add_llvm_definitions(-fPIC)
        set(ENABLE_PIC 1)
    else( SUPPORTS_FPIC_FLAG )
        message(STATUS "Warning: -fPIC not supported.")
    endif()
endif()

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LLVM_TOOLS_BINARY_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LLVM_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LLVM_BINARY_DIR}/lib )

add_llvm_definitions( -D__STDC_LIMIT_MACROS )
add_llvm_definitions( -D__STDC_CONSTANT_MACROS )

if( CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT WIN32 )
    # TODO: support other platforms and toolchains.
    option(LLVM_BUILD_32_BITS "Build 32 bits executables and libraries." OFF)
    if( LLVM_BUILD_32_BITS )
        message(STATUS "Building 32 bits executables and libraries.")
        add_llvm_definitions( -m32 )
        list(APPEND CMAKE_EXE_LINKER_FLAGS -m32)
        list(APPEND CMAKE_SHARED_LINKER_FLAGS -m32)
    endif( LLVM_BUILD_32_BITS )
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT WIN32 )

if( MSVC )
    # List of valid CRTs for MSVC
    set(MSVC_CRT
            MD
            MDd)

    set(LLVM_USE_CRT "" CACHE STRING "Specify VC++ CRT to use for debug/release configurations.")
    add_llvm_definitions( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS )
    add_llvm_definitions( -D_SCL_SECURE_NO_WARNINGS -DCRT_NONSTDC_NO_WARNINGS )
    add_llvm_definitions( -D_SCL_SECURE_NO_DEPRECATE )
    add_llvm_definitions( -wd4146 -wd4503 -wd4996 -wd4800 -wd4244 -wd4624 )
    add_llvm_definitions( -wd4355 -wd4715 -wd4180 -wd4345 -wd4224 )

    # Suppress 'new behavior: elements of array 'array' will be default initialized'
    add_llvm_definitions( -wd4351 )

    if (NOT ${LLVM_USE_CRT} STREQUAL "")
        list(FIND MSVC_CRT ${LLVM_USE_CRT} idx)
        if (idx LESS 0)
            message(FATAL_ERROR "Invalid value for LLVM_USE_CRT: ${LLVM_USE_CRT}. Valid options are one of: ${MSVC_CRT}")
        endif (idx LESS 0)
        add_llvm_definitions("/${LLVM_USE_CRT}")
        message(STATUS "Using VC++ CRT: ${LLVM_USE_CRT}")
    endif (NOT ${LLVM_USE_CRT} STREQUAL "")
endif( MSVC )

include_directories( ${LLVM_BINARY_DIR}/include ${LLVM_MAIN_INCLUDE_DIR})

#include(AddLLVM)
include(TableGen)

add_subdirectory(lib)

#set(LLVM_TABLEGEN "tblgen" CACHE
#        STRING "Native TableGen executable. Saves building one when cross-compiling.")
## Effective tblgen executable to be used:
#set(LLVM_TABLEGEN_EXE ${LLVM_TABLEGEN})

#add_subdirectory(utils/TableGen)
#
#set(LLVM_ENUM_ASM_PRINTERS "")
#set(LLVM_ENUM_ASM_PARSERS "")
#foreach(t ${LLVM_TARGETS_TO_BUILD})
#    message(STATUS "Targeting ${t}")
#    add_subdirectory(lib/Target/${t})
#    add_subdirectory(lib/Target/${t}/TargetInfo)
#    if( EXISTS ${LLVM_MAIN_SRC_DIR}/lib/Target/${t}/AsmPrinter/CMakeLists.txt )
#        add_subdirectory(lib/Target/${t}/AsmPrinter)
#        set(LLVM_ENUM_ASM_PRINTERS
#                "${LLVM_ENUM_ASM_PRINTERS}LLVM_ASM_PRINTER(${t})\n")
#    endif( EXISTS ${LLVM_MAIN_SRC_DIR}/lib/Target/${t}/AsmPrinter/CMakeLists.txt )
#    if( EXISTS ${LLVM_MAIN_SRC_DIR}/lib/Target/${t}/AsmParser/CMakeLists.txt )
#        add_subdirectory(lib/Target/${t}/AsmParser)
#        set(LLVM_ENUM_ASM_PARSERS
#                "${LLVM_ENUM_ASM_PARSERS}LLVM_ASM_PARSER(${t})\n")
#    endif( EXISTS ${LLVM_MAIN_SRC_DIR}/lib/Target/${t}/AsmParser/CMakeLists.txt )
#    set(CURRENT_LLVM_TARGET)
#endforeach(t)
#
## Produce llvm/Config/AsmPrinters.def
#configure_file(
#        ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/AsmPrinters.def.in
#        ${LLVM_BINARY_DIR}/include/llvm/Config/AsmPrinters.def
#)
#
## Produce llvm/Config/AsmParsers.def
#configure_file(
#        ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/AsmParsers.def.in
#        ${LLVM_BINARY_DIR}/include/llvm/Config/AsmParsers.def
#)

#add_subdirectory(lib/Target)