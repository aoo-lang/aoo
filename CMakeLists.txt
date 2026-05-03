cmake_minimum_required(VERSION "4.0")

project("aoo")

#------------Basic-----------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_FIND_USE_PACKAGE_REGISTRY FALSE)
set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY FALSE)
set(CMAKE_SUPPRESS_REGENERATION ON CACHE BOOL "" FORCE)
#----------------------------------

#--------------Tweaks--------------
include(CheckIPOSupported)
check_ipo_supported(RESULT linkTimeOptimizationSupported)
if(linkTimeOptimizationSupported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_library(ScopedFlags INTERFACE)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    message("Using MSVC / clang-cl")
    target_compile_options(ScopedFlags INTERFACE
        $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
        $<$<COMPILE_LANGUAGE:CXX>:/Zc:preprocessor>
        $<$<COMPILE_LANGUAGE:CXX>:/utf-8>
    )
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message("Using clang / clang-cl")
    add_compile_options(
        # I hate clang
        # —Yes I know about `-Wno-error` but I don't want to disable all warnings as errors.
        # If you run into some warnings that does not exist in this list, please add it here via PR.
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-unused-command-line-argument>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-unqualified-std-cast-call>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-unused-variable>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-macro-redefined>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-missing-field-initializers>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-invalid-offsetof>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-unused-function>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-unused-private-field>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-unused-but-set-variable>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-missing-braces>
        $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-macro-redefined>
    )
endif()

if(WIN32)
    #Override everything to Windows 10 SDK
    add_compile_options(
        $<$<COMPILE_LANGUAGE:C,CXX>:/U_WIN32_WINNT>
        $<$<COMPILE_LANGUAGE:C,CXX>:/UWINVER>
        $<$<COMPILE_LANGUAGE:C,CXX>:/UNTDDI_VERSION>
    )
    add_compile_definitions(
        $<$<COMPILE_LANGUAGE:C,CXX>:WIN32_LEAN_AND_MEAN>
        $<$<COMPILE_LANGUAGE:C,CXX>:NOMINMAX>
        $<$<COMPILE_LANGUAGE:C,CXX>:_WIN32_WINNT=0x0A00>
        $<$<COMPILE_LANGUAGE:C,CXX>:WINVER=0x0A00>
        $<$<COMPILE_LANGUAGE:C,CXX>:NTDDI_VERSION=0x0A000000>
    )
endif()
#----------------------------------

#------------Clangd----------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
#----------------------------------

#------------Files-----------------
file(GLOB_RECURSE AOO_SOURCES CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
#----------------------------------

#------------Dep:3rdparty----------
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "")
set(ZLIB_BUILD_SHARED  OFF CACHE BOOL "")
set(ZLIB_BUILD_STATIC  ON  CACHE BOOL "")
set(ZLIB_INSTALL       OFF CACHE BOOL "")
set(ZLIB_PREFIX        OFF CACHE BOOL "")
add_subdirectory("libs/zlib" EXCLUDE_FROM_ALL)
set(ZLIB_INCLUDE_DIR 
    "${CMAKE_SOURCE_DIR}/libs/zlib" 
    "${CMAKE_BINARY_DIR}/libs/zlib" 
    CACHE PATH "" FORCE
)
set(ZLIB_LIBRARY zlibstatic CACHE STRING "" FORCE)
target_include_directories(zlibstatic PUBLIC
    "${PROJECT_SOURCE_DIR}/libs/zlib"
    "${PROJECT_BINARY_DIR}/libs/zlib"
)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/src/llvm-patch")

#todo:fixme: Gemini suggested arguments. DYOR.
set(LLVM_ENABLE_DIA_SDK             OFF CACHE BOOL "" FORCE)
set(LLVM_TARGETS_TO_BUILD "X86;AArch64" CACHE STRING "Restrict build to target architectures")
set(LLVM_ENABLE_PROJECTS             "" CACHE STRING "" FORCE)
set(LLVM_INCLUDE_TESTS              OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_EXAMPLES           OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_BENCHMARKS         OFF CACHE BOOL "" FORCE)
set(LLVM_ENABLE_BINDINGS            OFF CACHE BOOL "" FORCE)
set(LLVM_ENABLE_RTTI                ON  CACHE BOOL "" FORCE)
set(LLVM_ENABLE_EH                  ON  CACHE BOOL "" FORCE)
add_subdirectory("libs/llvm/llvm" EXCLUDE_FROM_ALL)
#----------------------------------

#---------------AOO----------------
add_executable(${PROJECT_NAME} ${AOO_SOURCES})
#----------------------------------

#------------Build:Macro-----------
target_compile_definitions(${PROJECT_NAME} PRIVATE
    #Actual debug flag for AOO
    $<IF:$<CONFIG:Debug>,AOO_DEBUG,AOO_RELEASE>
)
#----------------------------------

#------------Inclusions------------
target_include_directories(${PROJECT_NAME} PRIVATE
    "libs/boost/assert/include"
    "libs/boost/config/include"
    "libs/boost/container_hash/include"
    "libs/boost/core/include"
    "libs/boost/describe/include"
    "libs/boost/mp11/include"
    "libs/boost/predef/include"
    "libs/boost/static_assert/include"
    "libs/boost/throw_exception/include"
    "libs/boost/unordered/include"

    "libs/cli11/include"

    "libs/llvm/llvm/include"
)
#----------------------------------

#---------------Link---------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    ScopedFlags

    zlibstatic

    LLVMCore
    LLVMSupport
    LLVMX86CodeGen
    LLVMX86Desc
    LLVMX86Info
)
#----------------------------------

#------------Link:System-----------
if(WIN32)

elseif(UNIX AND NOT APPLE)

elseif(APPLE)

endif()
#----------------------------------