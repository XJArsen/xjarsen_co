set(CO_ASYNC_DEBUG ON)

cmake_minimum_required(VERSION 3.10)
project(xjarsen_co VERSION 0.1.0 LANGUAGES C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置构建类型为 Debug
set(CMAKE_BUILD_TYPE Debug)

# 添加第三方库子目录
add_subdirectory(3rd/liburing)

# 添加可执行文件并包含源文件
add_executable(xjarsen_co main.cpp)

# 添加头文件路径
target_include_directories(xjarsen_co PUBLIC .)

# 确保输出目录存在
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 设置可执行文件输出目录
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin) 

# # 链接 liburing 库
# target_link_libraries(xjarsen_co PRIVATE uring)
