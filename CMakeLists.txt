cmake_minimum_required(VERSION 3.5)
find_package(LLVM REQUIRED CONFIG 6.0)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS} /usr/lib /usr/include)

add_subdirectory(interp)