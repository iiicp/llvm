# See docs/CMake.html for instructions about how to build LLVM with CMake.
cmake_minimum_required(VERSION 3.10)
project(LLVM)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PACKAGE_NAME llvm)
set(PACKAGE_VERSION 3.0)
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

if( NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR )
    file(GLOB_RECURSE
            tablegenned_files_on_include_dir
            "${LLVM_MAIN_SRC_DIR}/include/llvm/*.gen")
    file(GLOB_RECURSE
            tablegenned_files_on_lib_dir
            "${LLVM_MAIN_SRC_DIR}/lib/Target/*.inc")
    if( tablegenned_files_on_include_dir OR tablegenned_files_on_lib_dir)
        message(FATAL_ERROR "Apparently there is a previous in-source build,
probably as the result of running `configure' and `make' on
${LLVM_MAIN_SRC_DIR}.
This may cause problems. The suspicious files are:
${tablegenned_files_on_lib_dir}
${tablegenned_files_on_include_dir}
Please clean the source directory.")
    endif()
endif()

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

# List of targets with JIT support:
set(LLVM_TARGETS_WITH_JIT X86 PowerPC ARM Mips)

if( MSVC )
    set(LLVM_TARGETS_TO_BUILD X86
            CACHE STRING "Semicolon-separated list of targets to build, or \"all\".")
else( MSVC )
#    set(LLVM_TARGETS_TO_BUILD ${LLVM_ALL_TARGETS}
#            CACHE STRING "Semicolon-separated list of targets to build, or \"all\".")
    set(LLVM_TARGETS_TO_BUILD X86
        CACHE STRING "Semicolon-separated list of targets to build, or \"all\".")
endif( MSVC )

option(LLVM_ENABLE_CBE_PRINTF_A "Set to ON if CBE is enabled for printf %a output" ON)
if(LLVM_ENABLE_CBE_PRINTF_A)
    set(ENABLE_CBE_PRINTF_A 1)
endif()

option(LLVM_ENABLE_TIMESTAMPS "Enable embedding timestamp information in build" ON)
if(LLVM_ENABLE_TIMESTAMPS)
    set(ENABLE_TIMESTAMPS 1)
endif()

option(LLVM_ENABLE_FFI "Use libffi to call external functions from the interpreter" OFF)
set(FFI_LIBRARY_DIR "" CACHE PATH "Additional directory, where CMake should search for libffi.so")
set(FFI_INCLUDE_DIR "" CACHE PATH "Additional directory, where CMake should search for ffi.h or ffi/ffi.h")

set(LLVM_TARGET_ARCH "host"
        CACHE STRING "Set target to use for LLVM JIT or use \"host\" for automatic detection.")

option(LLVM_ENABLE_THREADS "Use threads if available." ON)

if( LLVM_TARGETS_TO_BUILD STREQUAL "all" )
    set( LLVM_TARGETS_TO_BUILD ${LLVM_ALL_TARGETS} )
endif()

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

include(AddLLVMDefinitions)

option(LLVM_ENABLE_PIC "Build Position-Independent Code" ON)

# MSVC has a gazillion warnings with this.
if( MSVC )
    option(LLVM_ENABLE_WARNINGS "Enable compiler warnings." OFF)
else( MSVC )
    option(LLVM_ENABLE_WARNINGS "Enable compiler warnings." ON)
endif()

option(LLVM_ENABLE_PEDANTIC "Compile with pedantic enabled." ON)
option(LLVM_ENABLE_WERROR "Fail and stop if a warning is triggered." OFF)

if( uppercase_CMAKE_BUILD_TYPE STREQUAL "RELEASE" )
    option(LLVM_ENABLE_ASSERTIONS "Enable assertions" OFF)
else()
    option(LLVM_ENABLE_ASSERTIONS "Enable assertions" ON)
endif()

include(config-ix)
include(HandleLLVMOptions)
configure_file(
        ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/config.h.cmake
        ${LLVM_BINARY_DIR}/include/llvm/Config/config.h)

configure_file(
        ${LLVM_MAIN_INCLUDE_DIR}/llvm/Config/llvm-config.h.cmake
        ${LLVM_BINARY_DIR}/include/llvm/Config/llvm-config.h)

configure_file(
        ${LLVM_MAIN_INCLUDE_DIR}/llvm/Support/DataTypes.h.cmake
        ${LLVM_BINARY_DIR}/include/llvm/Support/DataTypes.h)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${LLVM_TOOLS_BINARY_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LLVM_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LLVM_BINARY_DIR}/lib )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( ${LLVM_BINARY_DIR}/include ${LLVM_MAIN_INCLUDE_DIR})

if( ${CMAKE_SYSTEM_NAME} MATCHES SunOS )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include llvm/Support/Solaris.h")
endif( ${CMAKE_SYSTEM_NAME} MATCHES SunOS )

include(AddLLVM)
include(TableGen)

macro(llvm_tablegen)
    tablegen(LLVM ${ARGN})
endmacro()

if( MINGW )
    # People report that -O3 is unreliable on MinGW. The traditional
    # build also uses -O2 for that reason:
    llvm_replace_compiler_option(CMAKE_CXX_FLAGS_RELEASE "-O3" "-O2")
endif()

add_subdirectory(lib/Support)

#set(LLVM_TABLEGEN "tblgen" CACHE
#        STRING "Native TableGen executable. Saves building one when cross-compiling.")
## Effective tblgen executable to be used:
#set(LLVM_TABLEGEN_EXE ${LLVM_TABLEGEN})

#add_subdirectory(utils/TableGen)

#add_subdirectory(include/llvm)
#add_subdirectory(lib/VMCore)
#add_subdirectory(lib/CodeGen)
#add_subdirectory(lib/CodeGen/SelectionDAG)
#add_subdirectory(lib/CodeGen/AsmPrinter)
#add_subdirectory(lib/Bitcode/Reader)
#add_subdirectory(lib/Bitcode/Writer)
#add_subdirectory(lib/Transforms/Utils)
#add_subdirectory(lib/Transforms/Instrumentation)
#add_subdirectory(lib/Transforms/Scalar)
#add_subdirectory(lib/Transforms/IPO)
#add_subdirectory(lib/Transforms/Hello)

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

add_subdirectory(cmake/modules)