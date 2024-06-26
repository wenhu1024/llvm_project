cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(toy)


# 设置编译命令导出
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

# 添加可执行文件
add_executable(toy driver.cpp lexer.cpp parser.cpp codegen.cpp)

# 添加 LLVM 相关的头文件路径和库
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 链接 LLVM 相关的库
llvm_map_components_to_libnames(llvm_libs Analysis
Core
ExecutionEngine
InstCombine
Object
OrcJIT
RuntimeDyld
ScalarOpts
Support
native)


target_link_libraries(toy ${llvm_libs})
