cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(kaleidoscope LANGUAGES CXX)
set(CMAKE_CXX_COMPILER "clang++")


# 查找 LLVM 库并包含配置文件
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# 添加可执行文件
add_executable(kaleidoscope 
    driver.cpp 
    lexer.cpp 
    parser.cpp 
    codegen.cpp
)

# 包含 LLVM 头文件
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 链接 LLVM 库
llvm_map_components_to_libnames(llvm_libs all)
target_link_libraries(kaleidoscope ${llvm_libs})
