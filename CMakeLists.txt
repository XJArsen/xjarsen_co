cmake_minimum_required(VERSION 3.5.0)
project(co_web VERSION 0.1.0 LANGUAGES C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 23)

# 设置构建类型为 Debug
set(CMAKE_BUILD_TYPE Debug)

# 添加头文件路径
include_directories(${CMAKE_SOURCE_DIR}/co)

# 确保输出目录存在
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 设置可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 添加可执行文件并包含源文件
add_executable(co_web main.cpp co/epoll_loop.cpp)
