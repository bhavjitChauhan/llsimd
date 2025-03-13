cmake_minimum_required(VERSION 3.20)
project(psimd)

# Set this to a valid LLVM installation dir
set(CMAKE_PREFIX_PATH "" CACHE PATH "LLVM installation directory")

# Add the location of LLVMConfig.cmake to CMake search paths (so that
# find_package can locate it)
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}/lib/cmake/llvm/")

find_package(LLVM CONFIG)
if("${LLVM_VERSION_MAJOR}" VERSION_LESS 19)
  message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 19 or above")
endif()

message(STATUS "Found LLVM: ${LLVM_DIR} (found version \"${LLVM_PACKAGE_VERSION}\")")

# psimd includes headers from LLVM - update the include paths accordingly
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

# Use the same C++ standard as LLVM does
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

# LLVM is normally built without RTTI. Be consistent with that.
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

add_library(psimd SHARED psimd.cpp)

# Allow undefined symbols in shared objects on Darwin (this is the default
# behaviour on Linux)
target_link_libraries(psimd
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")

add_custom_target(check COMMAND lit -v ${CMAKE_CURRENT_SOURCE_DIR}/tests DEPENDS psimd)
