cmake_minimum_required(VERSION 3.5)
find_package(LLVM REQUIRED CONFIG 6.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS} /usr/lib /usr/include)
link_directories(/usr/local/lib)

add_subdirectory(interp)
